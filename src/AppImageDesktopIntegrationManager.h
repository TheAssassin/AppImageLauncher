#ifndef APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H
#define APPIMAGELAUNCHER_APPIMAGEDESKTOPINTEGRATIONMANAGER_H

#include <glib.h>
#include <memory>
#include <stdexcept>
#include <QString>
#include <QDir>
#include <xdg-basedir.h>
#include "AppImageLauncherException.h"

/**
 * Groups together the all the desktop integration functions.
 */
class AppImageDesktopIntegrationManager {
    static QDir integratedAppImagesDir;
public:
    AppImageDesktopIntegrationManager();

    /**
     * Moves the AppImage file into integratedAppImagesDir.
     * Create a desktop file pointing the AppImage.
     * Update the desktop database and icons cache.
     *
     * @param appImagePath
     * @throw IntegrationFailed
     */
    void integrateAppImage(const QString& appImagePath);

    /**
     * Generate the path where should be deployed (copied) the AppImage file.
     * @param pathToAppImage
     * @return path where should be deployed the AppImage
     */
    static QString buildDeploymentPath(const QString& pathToAppImage);

    /**
     * Checks if required files for the AppImage desktop integration are already in place.
     * @param pathToAppImage
     * @return true if was already integrated, otherwise false.
     */
    static bool hasAlreadyBeenIntegrated(const QString& pathToAppImage);

    /**
     * Check if the pathToAppImage is below the integratedAppImagesDir.
     * @param pathToAppImage
     * @return true if the pathToAppImage is below the integratedAppImagesDir, otherwise false.
     */
    bool isPlacedInTheDefaultAppsDir(const QString& pathToAppImage);

    /**
     * Create a .Desktop file for the AppImage pointed by pathToAppImage.
     * If resolve collisions is set to true and there is at least one Desktop file with the same name.
     * The method will append an integer to the Application name. More than one Desktop file whit
     * numbers after it's name the number will be the successor of the grater in the previous entries.
     *
     * @param pathToAppImage
     * @param resolveCollisions
     * @return true on success, otherwise false
     * @throw IntegrationFailed if isn't possible to create the file.
     */
    bool installDesktopFile(const QString& pathToAppImage, bool resolveCollisions);

    /**
     * Replaces existent deployed version of an AppImage by the one pointed by pathToAppImage.
     *
     * @throw IntegrationFailed in case of failure
     * @param pathToAppImage
     */
    void updateAppImage(const QString& pathToAppImage);

    /**
     * Remove Desktop file and icon of the AppImage pointed by appImagePath
     * @param appImagePath
     * @throw AppImageIntegrationRemovalFailed in case of failure
     */
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

    /**
     * Generate an md5 checksum from the AppImage file
     * @param pathToAppImage
     * @return md5 checksum
     */
    static QString getAppImageDigestMd5(const QString& pathToAppImage);

    /**
     * Remove ".Desktop" and icons files related to AppImage that are no longer in the system.
     *
     * @param verbose
     * @return true on success, otherwise false
     */
    static bool cleanUpOldDesktopIntegrationResources(bool verbose = false);

private:

    /**
     * Tries to move an AppImage file.
     *
     * @param pathToAppImage
     * @param pathToIntegratedAppImage
     * @throw OverridingExistingAppImageFile if the target file already exists
     * @throw IntegrationFailed if the operation fails
     */
    void tryMoveAppImage(const QString& pathToAppImage, const QString& pathToIntegratedAppImage) const;


    /**
     * Find ".Desktop" files whit the same names in the system folders.
     *
     * @param currentNameEntry
     * @return a map of the file name as key and the value of the name field as value
     */
    QMap<QString, QString> findCollisions(const QString& currentNameEntry);


    /**
     * Difference this desktopFile from others with the same name in the system by adding an
     * monotonically increasing integer at the end of this.
     *
     * @param desktopFilePath
     * @param desktopFile
     * @param nameEntry
     */
    void resolveDesktopFileCollisions(const char* desktopFilePath, const std::shared_ptr<GKeyFile>& desktopFile,
                                      const gchar* nameEntry);

    /**
     * Read additional Desktop Actions translation to be used in the generated Desktop file.
     *
     * @param filePath
     * @param fileName
     * @return Json object
     */
    QJsonObject readTranslationsFile(const QString& filePath, const QString& fileName) const;
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
