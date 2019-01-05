// system includes
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <vector>

// library includes
#include <fuse.h>

// local includes
#include "error.h"
#include "fs.h"


int main(int argc, char *argv[]) {
    // sanity check
    // the FS must be run by regular users only
    // running as root is likely to create security holes
    if ((getuid() == 0) || (geteuid() == 0)) {
        fprintf(stderr, "Must not be run as root\n");
        return 1;
    }

    // See which version of fuse we're running
    std::cerr << "FUSE version: " << FUSE_MAJOR_VERSION << "." << FUSE_MINOR_VERSION << std::endl;

    // Perform some sanity checking on the command line:  make sure
    // there are enough arguments, and that neither of the last two
    // start with a hyphen (this will break if you actually have a
    // rootpoint or mountpoint whose name starts with a hyphen, but so
    // will a zillion other programs)
    if ((argc != 1)) {
        std::cerr << "Usage: " << argv[0] << std::endl;
        return 1;
    }

    std::shared_ptr<AppImageLauncherFS> fs;

    // create filesystem instance
    for (int i = 0; i < 2; i++) {
        try {
            fs = AppImageLauncherFS::getInstance();
        } catch (const AlreadyRunningError&) {
            std::cerr << "Another instance is running already" << std::endl;
            return 1;
        }
    }

    if (fs == nullptr) {
        std::cerr << "Failed to create filesystem instance" << std::endl;
        return 1;
    }

    std::cerr << "mountpoint: " << fs->mountpoint() << std::endl;

    std::cerr << "Starting FUSE filesystem" << std::endl;
    int fuse_stat = fs->run();
    std::cerr << "Shutting down FUSE filesystem" << std::endl;

    return fuse_stat;
}
