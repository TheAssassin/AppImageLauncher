#pragma once

// system includes
#include <stdexcept>

// base class
class AppImageLauncherFSError : public std::runtime_error {
public:
    explicit AppImageLauncherFSError(const std::string& msg = "") : runtime_error(msg) {}
};

class AlreadyRunningError : public AppImageLauncherFSError { using AppImageLauncherFSError::AppImageLauncherFSError; };
class CouldNotOpenFileError : public AppImageLauncherFSError { using AppImageLauncherFSError::AppImageLauncherFSError; };
class FileNotFoundError : public AppImageLauncherFSError { using AppImageLauncherFSError::AppImageLauncherFSError; };
class InvalidPathError : public AppImageLauncherFSError { using AppImageLauncherFSError::AppImageLauncherFSError; };

class AppImageAlreadyRegisteredError : public AppImageLauncherFSError {
private:
    int _id;

public:
    explicit AppImageAlreadyRegisteredError(int id) : _id(id) {};

public:
    int id() {
        return _id;
    }
};
