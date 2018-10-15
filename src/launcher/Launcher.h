//
// Created by alexis on 8/30/18.
//

#ifndef APPIMAGELAUNCHER_LAUNCHER_H
#define APPIMAGELAUNCHER_LAUNCHER_H

// system includes
#include <vector>

// library includes
#include <QString>

// local includes
#include "AppImageDesktopIntegrationManager.h"
#include "../trashbin.h"

class Launcher {
    QString appImagePath;
    std::vector<char *> args{};
    int appImageType{-1};

    TrashBin *trashBin{nullptr};
    AppImageDesktopIntegrationManager *integrationManager{nullptr};

public:
    const QString &getAppImagePath() const;

    void setAppImagePath(const QString &appImagePath);

    const std::vector<char *> &getArgs() const;

    void setArgs(const std::vector<char *> &args);

    int getAppImageType() const;

    void setIntegrationManager(AppImageDesktopIntegrationManager *integrationManager);

    void setTrashBin(TrashBin *trashBin);

    void inspectAppImageFile();

    bool shouldBeIgnored();

    void executeAppImage();

    void overrideAppImageIntegration();

public slots:
    void integrateAppImage();

private:
    void validateAppImageType();

    bool isAMountOrExtractOperation() const;
};

/* Exceptions that can be thrown from the Launcher methods. */

class PathNotSetError : public std::runtime_error {
public:
    explicit PathNotSetError(const std::string &what) : runtime_error(what) {}
};

class ExecutionFailedError : public std::runtime_error {
public:
    explicit ExecutionFailedError(const std::string &what) : runtime_error(what) {}
};


#endif //APPIMAGELAUNCHER_LAUNCHER_H
