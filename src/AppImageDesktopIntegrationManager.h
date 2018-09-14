//
// Created by alexis on 9/1/18.
//

#ifndef APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H
#define APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H

#include <glib.h>
#include <memory>
#include <stdexcept>
#include <QString>
#include <QDir>
#include <xdg-basedir.h>

class AppImageDesktopIntegrationManager {
    QDir integratedAppImagesDir;
public:
    AppImageDesktopIntegrationManager();

    void integrateAppImage(const QString &appImagePath);

    QString buildDeploymentPath(const QString &pathToAppImage);

    bool hasAlreadyBeenIntegrated(const QString &pathToAppImage);

    bool isPlacedInTheDefaultAppsDir(const QString &pathToAppImage);

    bool installDesktopFile(const QString &pathToAppImage, bool resolveCollisions);

    void updateAppImage(const QString &pathToAppImage);

    void removeAppImageIntegration(const QString &appImagePath);

    const QDir &getIntegratedAppImagesDir() const;

    const QString getIntegratedAppImagesDirPath() const;

    static bool updateDesktopDatabaseAndIconCaches();

    static QString getAppImageDigestMd5(const QString &pathToAppImage);

    bool cleanUpOldDesktopIntegrationResources(bool verbose);

private:

    void tryMoveAppImage(const QString &pathToAppImage, const QString &pathToIntegratedAppImage) const;


    QMap<QString, QString> findCollisions(const QString &currentNameEntry);

    void resolveDesktopFileCollisions(const char *desktopFilePath, const std::shared_ptr<GKeyFile> &desktopFile,
                                      const gchar *nameEntry);

    QJsonObject readTranslationsFile(const QString &filePath, const QString &string) const;
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
