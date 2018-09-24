//
// Created by alexis on 8/30/18.
//

#ifndef APPIMAGELAUNCHER_LAUNCHER_H
#define APPIMAGELAUNCHER_LAUNCHER_H

#include <vector>
#include <QString>
#include <QIcon>
#include <nlohmann/json.hpp>
#include "AppImageDesktopIntegrationManager.h"
#include "AppImageLauncherException.h"
#include "../trashbin.h"

class Launcher {
    QString appImagePath;
    std::vector<char*> args{};
    int appImageType{-1};

    TrashBin* trashBin{nullptr};
    AppImageDesktopIntegrationManager* integrationManager{nullptr};

public:
    const QString& getAppImagePath() const;

    void setAppImagePath(const QString& appImagePath);

    const std::vector<char*>& getArgs() const;

    void setArgs(const std::vector<char*>& args);

    int getAppImageType() const;

    void setIntegrationManager(AppImageDesktopIntegrationManager* integrationManager);

    void setTrashBin(TrashBin* trashBin);

    void inspectAppImageFile();

    bool shouldBeIgnored();

    bool isAppImageExecutable();

    void executeAppImage();

    void overrideAppImageIntegration();

    nlohmann::json getAppImageInfo();

    QIcon getAppImageIcon();

public slots:

    void integrateAppImage();

private:
    void validateAppImageType();

    bool isAMountOrExtractOperation() const;

    void tryToMakeAppImageFileExecutable(const QString& path) const;
};

/* Exceptions that can be thrown from the Launcher methods. */

class AppImageFilePathNotSet : public AppImageLauncherException {
};

class ExecutionFailed : public AppImageLauncherException {
public:
    explicit ExecutionFailed(const std::string& what) : AppImageLauncherException(what) {}
};


#endif //APPIMAGELAUNCHER_LAUNCHER_H
