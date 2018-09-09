//
// Created by alexis on 9/1/18.
//

#ifndef APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H
#define APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H


#include <stdexcept>
#include <QString>
#include <QDir>


class AppImageDesktopIntegrationManager {
    QDir integratedAppImagesDir;
public:
    AppImageDesktopIntegrationManager();

    bool isIntegrationRequired(const QString &appImagePath);

    void integrateAppImage(const QString &appImagePath);

    QString buildDeploymentPath(const QString& pathToAppImage);

    bool hasAlreadyBeenIntegrated(const QString& pathToAppImage);

    bool isPlacedInTheDefaultAppsDir(const QString &pathToAppImage);

    bool installDesktopFile(const QString& pathToAppImage, bool resolveCollisions);

    void updateAppImage(const QString& pathToAppImage);

    void removeAppImageIntegration(const QString &appImagePath);

    const QDir &getIntegratedAppImagesDir() const;

    const QString getIntegratedAppImagesDirPath() const;

    static bool updateDesktopDatabaseAndIconCaches();
private:
    void loadIntegratedAppImagesDestination();

    void tryMoveAppImage(const QString &pathToAppImage, const QString &pathToIntegratedAppImage) const;
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

class AppImageIntegrationRemovalFailed : public std::runtime_error {
public:
    explicit AppImageIntegrationRemovalFailed(const std::string &what) : runtime_error(what) {}
};

#endif //APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H
