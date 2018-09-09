//
// Created by alexis on 9/1/18.
//

#ifndef APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H
#define APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H

// system includes
#include <stdexcept>

// library includes
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

private:
    void loadIntegratedAppImagesDestination();
};


class FileNotFoundError : public std::runtime_error {
public:
    explicit FileNotFoundError(const std::string &what) : runtime_error(what) {}
};

class InvalidAppImageError : public std::runtime_error {
public:
    explicit InvalidAppImageError(const std::string &what) : runtime_error(what) {}
};

class IntegrationFailedError : public std::runtime_error {
public:
    explicit IntegrationFailedError(const std::string &what) : runtime_error(what) {}
};

class UnsupportedTypeError : public std::runtime_error {
public:
    explicit UnsupportedTypeError(const std::string &what) : runtime_error(what) {}
};

class OverridingExistingFileError : public std::runtime_error {
public:
    explicit OverridingExistingFileError(const std::string &what) : runtime_error(what) {}
};

class AppImageIntegrationRemovalFailed : public std::runtime_error {
public:
    explicit AppImageIntegrationRemovalFailed(const std::string &what) : runtime_error(what) {}
};

#endif //APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H
