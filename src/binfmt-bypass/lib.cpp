// system headers
#include <cstdio>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <vector>
#include <memory.h>
#include <memory>
#include <stdexcept>
#include <cassert>
#include <filesystem>

// own headers
#include "elf.h"
#include "logging.h"
#include "lib.h"
#include "binfmt-bypass-preload.h"

#ifdef PRELOAD_LIB_NAME_32BIT
    #include "binfmt-bypass-preload_32bit.h"
#endif

#define EXIT_CODE_FAILURE 0xff

bool copy_and_patch_runtime(int fd, const char* const appimage_filename, const ssize_t elf_size) {
    // copy runtime header into memfd "file"
    {
        const auto realfd = open(appimage_filename, O_RDONLY);
        std::vector<char> buffer(elf_size);
        // TODO: check for errors
        read(realfd, buffer.data(), elf_size);
        write(fd, buffer.data(), elf_size);
        close(realfd);
    }

    // erase magic bytes
    lseek(fd, 8, SEEK_SET);
    char null_buf[]{0, 0, 0};
    write(fd, null_buf, 3);

    // TODO: handle errors properly
    return true;
}

#ifdef HAVE_MEMFD_CREATE

// if memfd_create is available, we should use it as it has a few important advantages over older solutions like
// shm_open or classic tempfiles

int create_memfd_with_patched_runtime(const char* const appimage_filename, const ssize_t elf_size) {
    // as we call exec() after fork() to create a child process (the parent keeps it alive, the child doesn't require
    // access anyway), we enable close-on-exec
    const auto memfd = memfd_create("runtime", MFD_CLOEXEC);

    if (memfd < 0) {
        log_error("memfd_create failed: %s\n", strerror(errno));
        return -1;
    }

    if (!copy_and_patch_runtime(memfd, appimage_filename, elf_size)) {
        log_error("failed to copy and patch runtime\n");
        close(memfd);
        return -1;
    }

    return memfd;
}

#else

// in case memfd_create is *not* available, we fall back to shm_open
// it requires a few more lines of code (e.g., changing permissions to limit access to the created file)
// also, we can't just

int create_shm_fd_with_patched_runtime(const char* const appimage_filename, const ssize_t elf_size) {
    // let's hope that mktemp returns a unique filename; if not, shm_open returns an error, thanks to O_EXCL
    // the file exists only for a fraction of a second normally, so the chances are not too bad
    char mktemp_template[] = "runtime-XXXXXX";
    const char* runtime_filename = mktemp(mktemp_template);

    if (runtime_filename[0] == '\0') {
        log_error("failed to create temporary filename\n");
        return -1;
    }

    // shm_open doesn't survive over exec(), so we _have to_ keep this process alive and create a child for the runtime
    // the good news is: we don't have to worry about setting flags to close-on-exec
    int writable_fd = shm_open(runtime_filename, O_RDWR | O_CREAT, 0700);

    if (writable_fd < 0) {
        log_error("shm_open failed (writable): %s\n", strerror(errno));
        return -1;
    }

    // open file read-only before unlinking the file, this is the fd we return later
    // otherwise we'll end up with ETXTBSY when trying to exec() it
    int readable_fd = shm_open(runtime_filename, O_RDONLY, 0);

    if (readable_fd < 0) {
        log_error("shm_open failed (read-only): %s\n", strerror(errno));
        return -1;
    }

    // let's make sure the file goes away when it's closed
    // as long as we don't close the fd, it won't go away, but if we do, the OS takes care of freeing the memory
    if (shm_unlink(runtime_filename) != 0) {
        log_error("shm_unlink failed: %s\n", strerror(errno));
        close(writable_fd);
        return -1;
    }

    if (!copy_and_patch_runtime(writable_fd, appimage_filename, elf_size)) {
        log_error("failed to copy and patch runtime\n");
        close(writable_fd);
        return -1;
    }

    // close writable fd and return readable one
    close(writable_fd);
    return readable_fd;
}

#endif

std::filesystem::path find_preload_library(bool is_32bit) {
    // packaging is now done using ld-p-native_packages which does not make guarantees about the install path
    // therefore, we need to look up the path of the preload library in relation to the current binary's path
    // since we use the F (fix binary) binfmt mode nowadays to enable the use of the interpreter in different cgroups,
    // namespaces or changeroots, we may not find the library there, but we'll at least try

    // we expect the library to be placed next to this binary
    const auto own_binary_path = std::filesystem::read_symlink("/proc/self/exe");
    const auto dir_path = own_binary_path.parent_path();

    std::filesystem::path rv = dir_path;

#ifdef PRELOAD_LIB_NAME_32BIT
    if (is_32bit) {
        rv /= PRELOAD_LIB_NAME_32BIT;
        return rv;
    }
#endif

    rv /= PRELOAD_LIB_NAME;
    return rv;
}

/**
 * Create a temporary file within the shm file system and maintain its existence using the RAII principle.
 * This is a first attempt, creating the files within /tmp. Future versions could try to put the files next to the
 * AppImage, use a reproducible path to create the lib file just once, use shm_open etc.
 */
class TemporaryPreloadLibFile {
public:
    TemporaryPreloadLibFile(const unsigned char* libContents, const std::streamsize libContentsSize) {
        char tempFilePattern[] = "/tmp/appimagelauncher-preload-XXXXXX.so";

        _fd = mkstemps(tempFilePattern, 3);
        if (_fd == -1) {
            throw std::runtime_error("could not create temporary preload lib file");
        }

        _path = tempFilePattern;

        if (write(_fd, libContents, libContentsSize) != libContentsSize) {
            throw std::runtime_error("failed to write contents to temporary preload lib");
        }
    }

    ~TemporaryPreloadLibFile() {
        close(_fd);
        unlink(_path.c_str());
    };

    std::string path() {
        return _path;
    }

private:
    int _fd;
    std::filesystem::path _path;
};

// need to keep track of the subprocess pid in a global variable, as signal handlers in C(++) are simple interrupt
// handlers that are not aware of any state in main()
// note that we only connect the signal handler once we have created a subprocess, i.e., we don't need to worry about
// subprocess_pid not being set yet
// it's best practice to implement a check anyway, though
static pid_t subprocess_pid = 0;

void forwardSignal(int signal) {
    if (subprocess_pid != 0) {
        log_debug("forwarding signal %d to subprocess %ld\n", signal, subprocess_pid);
        kill(subprocess_pid, signal);
    } else {
        log_error("signal %d received but no subprocess created yet, shutting down\n", signal);
        exit(signal);
    }
}

int bypassBinfmtAndRunAppImage(const std::string& appimage_path, const std::vector<char*>& target_args) {
    // read size of AppImage runtime (i.e., detect size of ELF binary)
    const auto size = elf_binary_size(appimage_path.c_str());

    if (size < 0) {
        log_error("failed to detect runtime size\n");
        return EXIT_CODE_FAILURE;
    }

#ifdef HAVE_MEMFD_CREATE
    // create "file" in memory, copy runtime there and patch out magic bytes
    const int runtime_fd = create_memfd_with_patched_runtime(appimage_path.c_str(), size);
#else
    const int runtime_fd = create_shm_fd_with_patched_runtime(appimage_filename.c_str(), size);
#endif

    if (runtime_fd < 0) {
        log_error("failed to set up in-memory file with patched runtime\n");
        return EXIT_CODE_FAILURE;
    }

    // to keep alive the memfd, we launch the AppImage as a subprocess
    if ((subprocess_pid = fork()) == 0) {
        // create new argv array, using passed filename as argv[0]
        std::vector<char*> new_argv;

        new_argv.push_back(strdup(appimage_path.c_str()));

        // insert remaining args, if any
        for (const auto& arg : target_args) {
            new_argv.push_back(strdup(arg));
        }

        // needs to be null terminated, of course
        new_argv.push_back(nullptr);

        // preload our library
        auto preload_lib_path = find_preload_library(is_32bit_elf(appimage_path));

        log_debug("preload lib path: %s\n", preload_lib_path.string().c_str());

        // may or may not be used, but must survive until this application terminates
        std::unique_ptr<TemporaryPreloadLibFile> temporaryPreloadLibFile;

        if (!std::filesystem::exists(preload_lib_path)) {
            log_warning("could not find preload library, creating new temporary file for it\n");

#ifdef PRELOAD_LIB_NAME_32BIT
            if (is_32bit_elf(appimage_path)) {
                temporaryPreloadLibFile = std::make_unique<TemporaryPreloadLibFile>(
                    libbinfmt_bypass_preload_32bit_so,
                    libbinfmt_bypass_preload_32bit_so_len
                );
            }
#endif

            if (temporaryPreloadLibFile == nullptr) {
                temporaryPreloadLibFile = std::make_unique<TemporaryPreloadLibFile>(
                    libbinfmt_bypass_preload_so,
                    libbinfmt_bypass_preload_so_len
                );
            }

            assert(temporaryPreloadLibFile != nullptr);

            preload_lib_path = temporaryPreloadLibFile->path();
        }

        if (!is_statically_linked_elf(appimage_path)) {
            log_debug("library to preload: %s\n", preload_lib_path.string().c_str());
            setenv("LD_PRELOAD", preload_lib_path.c_str(), true);
        }

        // calculate absolute path to AppImage, for use in the preloaded lib
        char* abs_appimage_path = realpath(appimage_path.c_str(), nullptr);
        log_debug("absolute AppImage path: %s\n", abs_appimage_path);
        // TARGET_APPIMAGE is further needed for static runtimes which do not make any use of LD_PRELOAD
        setenv("REDIRECT_APPIMAGE", abs_appimage_path, true);
        setenv("TARGET_APPIMAGE", abs_appimage_path, true);

        // launch memfd directly, no path needed
        log_debug("fexecve(...)\n");
        fexecve(runtime_fd, new_argv.data(), environ);

        log_error("failed to execute patched runtime: %s\n", strerror(errno));
        return EXIT_CODE_FAILURE;
    }

    // now that we have a subprocess and know its process ID, it's time to set up signal forwarding
    // note that from this point on, we don't handle signals ourselves any more, but rely on the subprocess to exit
    // properly
    for (int i = 0; i < 32; ++i) {
        signal(i, forwardSignal);
    }

    // wait for child process to exit, and exit with its return code
    int status;
    wait(&status);

    // clean up
    close(runtime_fd);

    // calculate return code based on child's behavior
    int child_retcode;

    if (WIFSIGNALED(status) != 0) {
        child_retcode = WTERMSIG(status);
        log_error("child exited with code %d\n", child_retcode);
    } else if (WIFEXITED(status) != 0) {
        child_retcode = WEXITSTATUS(status);
        log_debug("child exited normally with code %d\n", child_retcode);
    } else {
        log_error("unknown error: child didn't exit with signal or regular exit code\n");
        child_retcode = EXIT_CODE_FAILURE;
    }

    return child_retcode;
}
