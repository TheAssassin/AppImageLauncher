// system headers
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <memory.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <stdbool.h>

// own headers
#include "logging.h"

// saw this trick somewhere on the Internet... don't recall where it was, but it works well
#ifndef RTLD_NEXT
#define RTLD_NEXT ((void*) -1l)
#endif

#define REAL_LIBC RTLD_NEXT

// TODO: move into central header, it's the same value in main.cpp
#define EXIT_CODE_FAILURE 0xff

// pointers to actual implementations in libc
// will be initialized by __init()
// TODO: create a macro for this pattern (DRY)
static char* (*__libc_realpath)(const char*, char*) = NULL;
static int (*__libc_open)(const char*, int) = NULL;
static ssize_t (*__libc_readlink)(const char*, void*, size_t) = NULL;

// TODO: write __init() and call that from all functions, loading all required symbols, the AppImage path etc. once
// to improve performance

// DRY
static const char proc_self_exe[] = "/proc/self/exe";


void __init() {
    static bool initialized = false;

    if (!initialized) {
        initialized = true;

        // get rid of $LD_PRELOAD in the first binary which this library is preloaded into (should be the runtime)
        // the easiest way is to wait for one of these functions to be used, then unset it
        unsetenv("LD_PRELOAD");

        // load symbols from libc
        __libc_readlink = (ssize_t (*) (const char*, void*, size_t)) dlsym(REAL_LIBC, "readlink");
        __libc_realpath = (char* (*) (const char*, char*)) dlsym(REAL_LIBC, "realpath");
        __libc_open = (int (*) (const char*, int)) dlsym(REAL_LIBC, "open");

        if (__libc_readlink == NULL || __libc_realpath == NULL || __libc_open == NULL) {
            log_error("failed to load symbol from libc\n");
            exit(EXIT_CODE_FAILURE);
        }
    }
}

char* __abs_appimage_path() {
    __init();

    static const char env_var_name[] = "REDIRECT_APPIMAGE";

    char* appimage_var = getenv(env_var_name);

    if (appimage_var == NULL || appimage_var[0] == '\0') {
        log_error("$%s not set\n", env_var_name);
        exit(EXIT_CODE_FAILURE);
    }

    // make path absolute if needed (best effort, it's better to pass an absolute value)
    if (appimage_var[0] != '/') {
        log_warning("$%s value is not absolute, trying to make it absolute\n", env_var_name);

        char* abspath = calloc(PATH_MAX, sizeof(char));

        if (readlink(appimage_var, abspath, sizeof(abspath)) == -1) {
            log_error("readlink failed on %s: %s\n", appimage_var, strerror(errno));
            exit(EXIT_CODE_FAILURE);
        }

        return abspath;
    }

    return strdup(appimage_var);
}

__attribute__((visibility ("default")))
extern ssize_t readlink(const char* path, char* buf, size_t len) {
    __init();

    log_debug("readlink %s, len %ld\n", path, len);

    if (strncmp(path, proc_self_exe, strlen(proc_self_exe)) == 0) {
        char* abspath = __abs_appimage_path();

        log_debug("redirecting readlink to %s\n", abspath);

        size_t ret = strlen(abspath);

        strncpy(buf, abspath, ret);

        log_debug("buf: %s, len: %ld\n", buf, ret);

        free(abspath);
        return ret;
    }

    return __libc_readlink(path, buf, len);
}

__attribute__((visibility ("default")))
extern char* realpath(const char* name, char* resolved) {
    __init();

    log_debug("realpath %s, %s\n", name, resolved);

    if (strncmp(name, proc_self_exe, strlen(proc_self_exe)) == 0) {
        char* appimage = __abs_appimage_path();

        log_debug("changing realpath destination to %s\n", appimage);

        if (resolved == NULL) {
            resolved = appimage;
        } else {
            strncpy(resolved, appimage, PATH_MAX);
            free(appimage);
        }

        return resolved;
    }

    char* retval = __libc_realpath(name, resolved);

    log_debug("realpath result: %s -> %s, retval %s\n", name, resolved, retval);

    return retval;
}

// used by squashfuse, specifically util.c/sqfs_fd_open
__attribute__((visibility ("default")))
extern int open(const char* file, int flags, ...) {
    __init();

    log_debug("open(%s, %d)\n", file, flags);

    char* abspath = NULL;

    if (strncmp(file, proc_self_exe, strlen(proc_self_exe)) == 0) {
        abspath = __abs_appimage_path();
        log_debug("redirecting open to %s\n", abspath);
        file = abspath;
    }

    int result = __libc_open(file, flags);

    if (abspath != NULL) {
        free(abspath);
    }

    return result;
}
