//
// Created by alexis on 8/30/18.
//

#include <QFile>
#include <appimage/appimage.h>
#include "Launcher.h"


const QString &Launcher::getAppImagePath() const {
    return appImagePath;
}

void Launcher::setAppImagePath(const QString &appImagePath) {
    Launcher::appImagePath = appImagePath;
}

const std::vector<char *> &Launcher::getArgs() const {
    return args;
}

void Launcher::setArgs(const std::vector<char *> &args) {
    Launcher::args = args;
}

int Launcher::getAppImageType() const {
    return appImageType;
}


void Launcher::inspectAppImageFile() {
    if (appImagePath.isEmpty())
        throw AppImageFilePathNotSet("");

    if (!QFile::exists(appImagePath))
        throw AppImageFileNotExists(appImagePath.toStdString());

    validateAppImageType();
}

void Launcher::validateAppImageType() {
    appImageType = appimage_get_type(appImagePath.toStdString().c_str(), false);
    if (appImageType < 1)
        throw InvalidAppImageFile("");

    if (appImageType > 2)
        throw UnsuportedAppImageType("");
}

bool Launcher::isIntegrationRequired() {
    return false;
}

void Launcher::integrateAppImage() {

}

void Launcher::executeAppImage() {

}

bool Launcher::shouldBeIgnored() {
    bool result =
            isAMountOrExtractOperation() &&
            // check for X-AppImage-Integrate=false
            appimage_shall_not_be_integrated(appImagePath.toStdString().c_str()) &&
            // ignore terminal apps (fixes #2)
            appimage_is_terminal_app(appImagePath.toStdString().c_str()) &&
            // AppImages in AppImages are not supposed to be integrated
            appImagePath.startsWith("/tmp/.mount_");

    return result;
}

bool Launcher::isAMountOrExtractOperation() const {
    // type 2 specific checks
    if (appImageType == 2) {
        // check parameters
        {
            for (auto &i : args) {
                QString arg = i;

                // reserved argument space
                const QString prefix = "--appimage-";

                if (arg.startsWith(prefix)) {
                    // don't annoy users who try to mount or extract AppImages
                    if (arg == prefix + "mount" || arg == prefix + "extract") {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


