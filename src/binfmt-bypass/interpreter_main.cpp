// system headers
#include <cassert>
#include <unistd.h>
#include <vector>

// own headers
#include "logging.h"
#include "lib.h"

bool executableExists(const std::string& path) {
    if (access(path.c_str(), X_OK) != 0) {
        log_debug("executable %s does not exist\n", path.c_str());
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    log_debug("Welcome to AppImageLauncher's binfmt_misc interpreter!\n");

    if (argc <= 1) {
        log_message("Usage: %s <AppImage file> [args...]\n", argv[0]);
        return EXIT_CODE_FAILURE;
    }

    const std::string appImagePath = argv[1];
    std::vector<char*> args(&argv[2], &argv[argc]);

    // optimistic approach
    bool useAppImageLauncher = true;

    if (!executableExists(APPIMAGELAUNCHER_PATH)) {
        log_message(
            "AppImageLauncher not found at %s, launching AppImage directly: %s\n",
            appImagePath.c_str()
        );
        useAppImageLauncher = false;
    }

    if (getenv("APPIMAGELAUNCHER_DISABLE") != nullptr) {
        log_message(
            "APPIMAGELAUNCHER_DISABLE set, launching AppImage directly: %s\n",
            appImagePath.c_str()
        );
        useAppImageLauncher = false;
    }

    if (!useAppImageLauncher) {
        return bypassBinfmtAndRunAppImage(argv[1], args);
    }

    log_debug(
        "AppImageLauncher found at %s, launching AppImage %s with it\n",
        APPIMAGELAUNCHER_PATH,
        appImagePath.c_str()
    );

    // needs to be done in inverse order
    args.emplace(args.begin(), strdup(appImagePath.c_str()));
    args.emplace(args.begin(), strdup(APPIMAGELAUNCHER_PATH));

    const auto rv = execv(APPIMAGELAUNCHER_PATH, args.data());

    assert(rv == -1);
    log_error("execv(%s, ...) failed\n");
    return EXIT_CODE_FAILURE;
}
