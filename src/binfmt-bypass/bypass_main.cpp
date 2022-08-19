// system headers
#include <vector>

// own headers
#include "logging.h"
#include "lib.h"

int main(int argc, char** argv) {
    if (argc <= 1) {
        log_message("Usage: %s <AppImage file> [args...]\n", argv[0]);
        return EXIT_CODE_FAILURE;
    }

    std::vector<char*> args(&argv[2], &argv[argc]);
    return bypassBinfmtAndRunAppImage(argv[1], args);
}
