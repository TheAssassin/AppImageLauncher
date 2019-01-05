#pragma once

// system includes
#include <memory>
#include <string>

// library includes
#include <fuse.h>

class AppImageLauncherFS {
private:
    class PrivateData;
    std::shared_ptr<PrivateData> d;

    static std::shared_ptr<AppImageLauncherFS> instance;

// this class is a singleton
// therefore, no public constructor, no copying, and no public constructor, but a getInstance() method
private:
    // constructor doesn't take any arguments
    AppImageLauncherFS();

    // private copy constructor = no copies
    AppImageLauncherFS(const AppImageLauncherFS&);
    // same goes for the assignment operator
    AppImageLauncherFS& operator=(const AppImageLauncherFS&);

public:
    static std::shared_ptr<AppImageLauncherFS> getInstance();

public:
    // returns calculated mountpoint directory path
    std::string mountpoint();

    // returns a FUSE style list of operations that can be passed to fuse_main etc.
    // you should prefer using run()
    std::shared_ptr<struct fuse_operations> operations() const;

    // runs filesystem with FUSE
    // returns exit code received from FUSE
    int run();
};
