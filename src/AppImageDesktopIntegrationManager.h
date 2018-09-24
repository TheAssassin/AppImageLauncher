#ifndef APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H
#define APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H

#include <glib.h>
#include <memory>
#include <stdexcept>
#include <QString>
#include <QDir>
#include <xdg-basedir.h>
#include "AppImageLauncherException.h"

class AppImageDesktopIntegrationManager {
    static QDir integratedAppImagesDir;
public:
    AppImageDesktopIntegrationManager();

    void integrateAppImage(const QString& appImagePath);

    static QString buildDeploymentPath(const QString& pathToAppImage);

    static bool hasAlreadyBeenIntegrated(const QString& pathToAppImage);

    bool isPlacedInTheDefaultAppsDir(const QString& pathToAppImage);

    bool installDesktopFile(const QString& pathToAppImage, bool resolveCollisions);

    void updateAppImage(const QString& pathToAppImage);

    void removeAppImageIntegration(const QString& appImagePath);

    const QDir& getIntegratedAppImagesDir() const;

    const QString getIntegratedAppImagesDirPath() const;

    /**
     * Description: Update desktop database and icon caches of desktop environments
     * this makes sure that:
     * - outdated entries are removed from the launcher
     * - icons of freshly integrated AppImages are displayed in the launcher
     **/
    static bool updateDesktopDatabaseAndIconCaches();

    static QString getAppImageDigestMd5(const QString& pathToAppImage);

    static bool cleanUpOldDesktopIntegrationResources(bool verbose);

private:

    void tryMoveAppImage(const QString& pathToAppImage, const QString& pathToIntegratedAppImage) const;


    QMap<QString, QString> findCollisions(const QString& currentNameEntry);

    void resolveDesktopFileCollisions(const char* desktopFilePath, const std::shared_ptr<GKeyFile>& desktopFile,
                                      const gchar* nameEntry);

    QJsonObject readTranslationsFile(const QString& filePath, const QString& string) const;
};


class AppImageFileNotExists : public AppImageLauncherException {
public:
    explicit AppImageFileNotExists(const std::string& what) : AppImageLauncherException(what) {}
};

class InvalidAppImageFile : public AppImageLauncherException {
public:
    explicit InvalidAppImageFile(const std::string& what) : AppImageLauncherException(what) {}
};

class IntegrationFailed : public AppImageLauncherException {
public:
    explicit IntegrationFailed(const std::string& what) : AppImageLauncherException(what) {}
};

class UnsuportedAppImageType : public AppImageLauncherException {
public:
    explicit UnsuportedAppImageType(const std::string& what) : AppImageLauncherException(what) {}
};

class OverridingExistingAppImageFile : public AppImageLauncherException {
public:
    explicit OverridingExistingAppImageFile(const std::string& what) : AppImageLauncherException(what) {}
};

class AppImageIntegrationRemovalFailed : public AppImageLauncherException {
public:
    explicit AppImageIntegrationRemovalFailed(const std::string& what) : AppImageLauncherException(what) {}
};

#endif //APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H
