// system headers
#include <cstdio>
#include <iostream>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <wait.h>
#include <vector>
#include <memory.h>
#include <libgen.h>

// own headers
#include "elf.h"
#include "logging.h"

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

char* find_preload_library(bool is_32bit) {
    // we expect the library to be placed next to this binary
    char* own_binary_path = realpath("/proc/self/exe", nullptr);

    if (own_binary_path == nullptr) {
        log_error("could not detect own binary's path");
        return nullptr;
    }

    char* dir_path = dirname(own_binary_path);

    if (dir_path == nullptr) {
        log_error("could not detect own binary's directory path");
    }

    auto path_size = strlen(dir_path) + 1 + strlen(PRELOAD_LIB_NAME) + 1;

#ifdef PRELOAD_LIB_NAME_32BIT
    path_size += strlen(PRELOAD_LIB_NAME_32BIT);
#endif

    char* result = static_cast<char*>(malloc(path_size));
    sprintf(result, "%s/", dir_path);

#ifdef PRELOAD_LIB_NAME_32BIT
    if (is_32bit) {
        strcat(result, PRELOAD_LIB_NAME_32BIT);
    } else {
        strcat(result, PRELOAD_LIB_NAME);
    }
#else
    strcat(result, PRELOAD_LIB_NAME);
#endif

    return result;
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        log_message("Usage: %s <AppImage file> [...]\n", argv[0]);
        return EXIT_CODE_FAILURE;
    }

    const auto* appimage_filename = argv[1];
    log_debug("AppImage filename: %s\n", appimage_filename);

    // read size of AppImage runtime (i.e., detect size of ELF binary)
    const auto size = elf_binary_size(appimage_filename);

    if (size < 0) {
        std::cerr << "failed to detect runtime size" << std::endl;
        return EXIT_CODE_FAILURE;
    }

#ifdef HAVE_MEMFD_CREATE
    // create "file" in memory, copy runtime there and patch out magic bytes
    int runtime_fd = create_memfd_with_patched_runtime(appimage_filename, size);
#else
    int runtime_fd = create_shm_fd_with_patched_runtime(appimage_filename, size);
#endif

    if (runtime_fd < 0) {
        log_error("failed to set up in-memory file with patched runtime\n");
        return EXIT_CODE_FAILURE;
    }

    // to keep alive the memfd, we launch the AppImage as a subprocess
    if (fork() == 0) {
        // create new argv array, using passed filename as argv[0]
        std::vector<char*> new_argv;

        new_argv.push_back(strdup(appimage_filename));

        // insert remaining args, if any
        for (int i = 2; i < argc; ++i) {
            new_argv.push_back(argv[i]);
        }

        // needs to be null terminated, of course
        new_argv.push_back(nullptr);

        // preload our library
        char* preload_lib_path = find_preload_library(is_32bit_elf(appimage_filename));

        if (preload_lib_path == nullptr) {
            log_error("could not find preload library path");
            return EXIT_CODE_FAILURE;
        }

        log_debug("library to preload: %s\n", preload_lib_path);

        setenv("LD_PRELOAD", preload_lib_path, true);

        // calculate absolute path to AppImage, for use in the preloaded lib
        char* abs_appimage_path = realpath(appimage_filename, nullptr);
        log_debug("absolute AppImage path: %s\n", abs_appimage_path);
        setenv("REDIRECT_APPIMAGE", abs_appimage_path, true);

        // launch memfd directly, no path needed
        log_debug("fexecve(...)\n");
        fexecve(runtime_fd, new_argv.data(), environ);

        log_error("failed to execute patched runtime: %s\n", strerror(errno));
        return EXIT_CODE_FAILURE;
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
        child_retcode = status;
        log_debug("child exited normally with code %d\n", child_retcode);
    } else {
        log_error("unknown error: child didn't exit with signal or regular exit code\n");
        child_retcode = EXIT_CODE_FAILURE;
    }

    return child_retcode;
}
