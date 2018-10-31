//
// Created by alexis on 8/30/18.
//

#ifndef APPIMAGELAUNCHER_LAUNCHER_H
#define APPIMAGELAUNCHER_LAUNCHER_H

// system includes
#include <vector>

// library includes
#include <QString>
#include <QIcon>
#include <nlohmann/json.hpp>
#include "AppImageDesktopIntegrationManager.h"
#include "../trashbin.h"

class Launcher {
private:
    QString appImagePath;
    std::vector<char *> args{};
    int appImageType{-1};

    TrashBin *trashBin{nullptr};
    AppImageDesktopIntegrationManager *integrationManager{nullptr};

public:
    Launcher(QString path, std::vector<char*> argv, AppImageDesktopIntegrationManager* mgr, TrashBin* bin);

public:
    void setAppImagePath(const QString& appImagePath);

    const QString& getAppImagePath() const;

    const std::vector<char*>& getArgs() const;

    int getAppImageType() const;

    bool shouldBeIgnored();

    bool isAppImageExecutable();

    void executeAppImage();

    void overrideAppImageIntegration();

    void validateAppImage() const;

    nlohmann::json getAppImageInfo();

    QIcon getAppImageIcon();

public slots:
    void integrateAppImage();

private:
    static void validateAppImage(const QString& path);

    bool isAMountOrExtractOperation() const;
};

/* Exceptions that can be thrown from the Launcher methods. */

class ValueError : public std::runtime_error {
public:
    explicit ValueError(const std::string &what) : runtime_error(what) {}
};

class ExecutionFailedError : public std::runtime_error {
public:
    explicit ExecutionFailedError(const std::string &what) : runtime_error(what) {}
};


#endif //APPIMAGELAUNCHER_LAUNCHER_H
