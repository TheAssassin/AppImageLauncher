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

int create_memfd_with_patched_runtime(const char* const appimage_filename, const ssize_t elf_size) {
    // as we call exec() after fork() to create a child process (the parent keeps it alive, the child doesn't require
    // access anyway), we enable close-on-exec
    const auto memfd = memfd_create("runtime", MFD_CLOEXEC);

    if (memfd < 0) {
        std::cerr << "memfd_create failed" << std::endl;
        return EXIT_CODE_FAILURE;
    }

    // copy runtime header into memfd "file"
    {
        const auto realfd = open(appimage_filename, O_RDONLY);
        std::vector<char> buffer(elf_size);
        // TODO: check for errors
        read(realfd, buffer.data(), elf_size);
        write(memfd, buffer.data(), elf_size);
        close(realfd);
    }

    // erase magic bytes
    lseek(memfd, 8, SEEK_SET);
    char null_buf[]{0, 0, 0};
    write(memfd, null_buf, 3);

    return memfd;
}

char* find_preload_library() {
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

    char* result = static_cast<char*>(malloc(strlen(dir_path) + 1 + strlen(PRELOAD_LIB_NAME) + 1));
    sprintf(result, "%s/%s", dir_path, PRELOAD_LIB_NAME);
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

    // create "file" in memory, copy runtime there and patch out magic bytes
    int memfd = create_memfd_with_patched_runtime(appimage_filename, size);

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
        char* preload_lib_path = find_preload_library();

        if (preload_lib_path == nullptr) {
            log_error("could not find preload library path");
            return EXIT_CODE_FAILURE;
        }

        setenv("LD_PRELOAD", preload_lib_path, true);

        // calculate absolute path to AppImage, for use in the preloaded lib
        char* abs_appimage_path = realpath(appimage_filename, nullptr);
        log_debug("absolute AppImage path: %s\n", abs_appimage_path);
        setenv("REDIRECT_APPIMAGE", abs_appimage_path, true);

        // launch memfd directly, no path needed
        log_debug("fexecve(...)\n");
        fexecve(memfd, new_argv.data(), environ);

        return EXIT_CODE_FAILURE;
    }

    // wait for child process to exit, and exit with its return code
    int status;
    wait(&status);

    // clean up
    close(memfd);

    return WEXITSTATUS(status);
}
