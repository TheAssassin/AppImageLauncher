#ifndef APPIMAGELAUNCHER_APPIMAGELAUNCHEREXCEPTION_H
#define APPIMAGELAUNCHER_APPIMAGELAUNCHEREXCEPTION_H

#include <stdexcept>

class AppImageLauncherException : public std::runtime_error {
public:
    AppImageLauncherException() : runtime_error("") {}

    explicit AppImageLauncherException(const std::string& what) : runtime_error(what) {}
};


#endif //APPIMAGELAUNCHER_APPIMAGELAUNCHEREXCEPTION_H
