#ifndef APPIMAGELAUNCHER_LAUNCHER_H
#define APPIMAGELAUNCHER_LAUNCHER_H

#include <vector>
#include <QString>
#include <QIcon>

#include <nlohmann/json.hpp>

#include <AppImageDesktopIntegrationManager.h>
#include <AppImageLauncherException.h>
#include <trashbin.h>

/**
 * Contains the main methods that support the AppImageLancher workflow.
 *
 * It doesn't include any kind of ui.
 * Exceptions are used to provide feedback about the failure of given operation.
 */
class Launcher {
    QString appImagePath;
    std::vector<char*> args{};
    int appImageType{-1};

    TrashBin* trashBin{nullptr};
    QSharedPointer<AppImageDesktopIntegrationManager> integrationManager;

public:
    /**
     * Set the path to the AppImage to be integrated.
     * Always required.
     *
     * @param appImagePath
     */
    void setAppImagePath(const QString& appImagePath);

    /**
     * Set the args to be used while executing the AppImage.
     * Always required.
     *
     * @param args
     */
    void setArgs(const std::vector<char*>& args);

    /**
     * Set the Desktop Integration manager to be used.
     * Always required.
     *
     * @param integrationManager
     */
    void setIntegrationManager(QSharedPointer<AppImageDesktopIntegrationManager> integrationManager);

    /**
     * Set the Trash Bin  implementation to be used.
     *
     * Always required.
     * @param trashBin
     */
    void setTrashBin(TrashBin* trashBin);

    /**
     * Check that a valid AppImage file is being pointed by appImagePath
     *
     * @see setAppImagePath(const QString& appImagePath)
     *
     * @throw InvalidAppImageFile if the file not an AppImage or it's corrupted
     * @throw UnsuportedAppImageType if the AppImage file format is not supported
     * @throw AppImageFileNotExists if the file doesn't exist or cannot be accessed
     * @throw AppImageFilePathNotSet
     * */
    void inspectAppImageFile();

    /**
     * Check if the Launcher workflow should be ignored.
     *
     * The cases in which this is valid are:
     *  - when performing mount or extract operations
     *  - when the AppImage author explicitly set that the AppImage should not be integrated
     *  - when it's a terminal application
     *  - when the appImagePath start with '"/tmp/.mount_' which means that it's provably inside
     *  another AppImage.
     *
     * @return true if the AppImage is a candidate for desktop integration. Otherwise returns false.
     */
    bool isCandidateForDesktopIntegration();

    /**
     * Check the AppImage file execution permissions
     * @return true if the file can be executed by the current user. Otherwise returns false.
     */
    bool isAppImageExecutable();

    /**
     * Execute an AppImage.
     *
     * As AppImageLauncher intercept all the open attempts of AppImage files by means of binfmt_misc.
     * We should prevent recursive invocations of AppImageLauncher therefore different approaches are
     * used depending on the AppImage type. See https://github.com/TheAssassin/AppImageLauncher/issues/3
     *
     * @throw InvalidAppImageFile
     * @throw UnsuportedAppImageType
     * @throw AppImageFileNotExists
     * @throw AppImageFilePathNotSet
     * @throw ExecutionFailed
     */
    void executeAppImage();

    /**
     * Read the metadata contained in the AppImage file.
     *
     * Mix the information from the following sources:
     *  - elf file header
     *  - application "Desktop" file
     *  - application "appstream.xml" file
     *  see https://github.com/azubieta/AppImageInfo/blob/master/output-example.json
     * @return Json structure containing the AppImage metadata
     */
    nlohmann::json getAppImageInfo();

    /**
     * Reads the AppImage DirIcon
     * @return AppImage DirIcon
     */
    QIcon getAppImageIcon();

    /**
     * Execute the integrateAppImage method from the AppImageDesktopIntegrationManager
     *
     * @throw IntegrationFailed if something goes wrong
     */
    void integrateAppImage();

    /**
     * Disposes any existent version of the AppImage into the TrashBin an execute the
     * integrateAppImage method from the AppImageDesktopIntegrationManager
     *
     * @throw IntegrationFailed if something goes wrong
     */
    void overrideAppImageIntegration();

private:
    /**
     * Check the AppImage type.
     *
     * @throw InvalidAppImageFile
     * @throw UnsuportedAppImageType
     */
    void validateAppImageType();

    /**
     * Check if the --appimage-mount or --appimage-extract arguments are present.
     *
     * @return true if they are. Otherwise false.
     */
    bool isAMountOrExtractOperation() const;

    /**
     * Grants read and execution permission to a given file.
     *
     * @param path
     * @throw ExecutionFailed if the user doesn't have enough permissions.
     */
    void tryToMakeAppImageFileExecutable(const QString& path) const;

    void disposeAppImageWithTheSameName() const;

    void disposeAppImageWithTheSameMD5() const;
};

/* Exceptions that can be thrown from the Launcher methods. */

class AppImageFilePathNotSet : public AppImageLauncherException {
};

class ExecutionFailed : public AppImageLauncherException {
public:
    explicit ExecutionFailed(const std::string& what) : AppImageLauncherException(what) {}
};


#endif //APPIMAGELAUNCHER_LAUNCHER_H
