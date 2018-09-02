//
// Created by alexis on 9/1/18.
//

#ifndef APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H
#define APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H


#include <stdexcept>
#include <QString>


class AppImageDesktopIntegrationManager {

public:
    bool isIntegrationRequired(const QString &appImagePath);

    void integrateAppImage(const QString &appImagePath);

    QString buildDeploymentPath(const QString& pathToAppImage);

    bool hasAlreadyBeenIntegrated(const QString& pathToAppImage);


    bool installDesktopFile(const QString& pathToAppImage, bool resolveCollisions);

    void updateAppImage(const QString& pathToAppImage);
};


class AppImageFileNotExists : public std::runtime_error {
public:
    explicit AppImageFileNotExists(const std::string &what) : runtime_error(what) {}
};

class InvalidAppImageFile : public std::runtime_error {
public:
    explicit InvalidAppImageFile(const std::string &what) : runtime_error(what) {}
};

class IntegrationFailed : public std::runtime_error {
public:
    explicit IntegrationFailed(const std::string &what) : runtime_error(what) {}
};

class UnsuportedAppImageType : public std::runtime_error {
public:
    explicit UnsuportedAppImageType(const std::string &what) : runtime_error(what) {}
};

class OverridingExistingAppImageFile : public std::runtime_error {
public:
    explicit OverridingExistingAppImageFile(const std::string &what) : runtime_error(what) {}
};
#endif //APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H
