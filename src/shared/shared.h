/* central file for utility functions */

#pragma once

// system headers
#include <string>
#include <memory>

// library headers
#include <QDir>
#include <QString>
#include <QSettings>

// local headers
#include "types.h"

enum IntegrationState {
    INTEGRATION_FAILED = 0,
    INTEGRATION_SUCCESSFUL,
    INTEGRATION_ABORTED
};

// standard location for integrated AppImages
// currently hardcoded, can not be changed by users
static const auto DEFAULT_INTEGRATION_DESTINATION = QString(getenv("HOME")) + "/Applications/";

// little convenience method to display warnings
void displayWarning(const QString& message);

// little convenience method to display errors
void displayError(const QString& message);

// reliable way to check if the current session is graphical or not
bool isHeadless();

// makes an existing file executable
bool makeExecutable(const QString& path);

// removes executable bits from file's permissions
bool makeNonExecutable(const QString& path);

#ifndef BUILD_LITE
// calculate path to private libdir, containing tools and libraries specific to and used by AppImageLauncher
QString privateLibDirPath(const QString& srcSubdirName);
#endif

// installs desktop file for given AppImage, including AppImageLauncher specific modifications
// set resolveCollisions to false in order to leave the Name entries as-is
bool installDesktopFileAndIcons(const QString& pathToAppImage, bool resolveCollisions = true);

// update AppImage's existing desktop file with AppImageLauncher specific entries
// this alias for installDesktopFileAndIcons does not perform any collision detection and resolving
bool updateDesktopFileAndIcons(const QString& pathToAppImage);

// update desktop database and icon caches of desktop environments
// this makes sure that:
//   - outdated entries are removed from the launcher
//   - icons of freshly integrated AppImages are displayed in the launcher
bool updateDesktopDatabaseAndIconCaches();

// integrates an AppImage using a standard workflow used across all AppImageLauncher applications
IntegrationState integrateAppImage(const QString& pathToAppImage, const QString& pathToIntegratedAppImage);

// write config file to standard location with given configuration values
// askToMove and enableDaemon both are bools but represented as int to add some sort of "unset" state
// < 0: unset; 0 = false; > 0 = true
// destination is a string that, when empty, will be interpreted as "use default"
void createConfigFile(int askToMove, const QString& destination, int enableDaemon,
                      const QStringList& additionalDirsToWatch = {}, int monitorMountedFilesystems = -1);

// replaces ~ character in paths with real home directory, if necessary and possible
QString expandTilde(QString path);

// load config file and return it
std::shared_ptr<QSettings> getConfig();

// return directory into which the integrated AppImages will be moved
QDir integratedAppImagesDestination();

// additional directories to monitor for AppImages, and to permit AppImages to be within (i.e., shall not ask whether
// to move to the main location, if they're in one of these, it's all good)
QSet<QString> additionalAppImagesLocations(bool includeValidMountPoints = false);

// calculate list of directories the daemon has to watch
// AppImages inside there should furthermore not be moved out of there and into the main integration directory
QDirSet daemonDirectoriesToWatch(const std::shared_ptr<QSettings>& config = nullptr);

// build path to standard location for integrated AppImages
QString buildPathToIntegratedAppImage(const QString& pathToAppImage);

// get AppImage MD5 digest
// extracts the digest embedded in the file
// if no such digest has been embedded, it calculates it using libappimage
QString getAppImageDigestMd5(const QString& path);

// checks whether AppImage has been integrated already
bool hasAlreadyBeenIntegrated(const QString& pathToAppImage);

// checks whether file is in a given directory
bool isInDirectory(const QString& pathToAppImage, const QDir& directory);

// clean up old desktop files (and related resources, such as icons)
bool cleanUpOldDesktopIntegrationResources(bool verbose = false);

// returns absolute path to currently running binary
std::shared_ptr<char> getOwnBinaryPath();

// returns true if AppImageLauncher was updated since the desktop file for a given AppImage has been updated last
bool desktopFileHasBeenUpdatedSinceLastUpdate(const QString& pathToAppImage);

// checks whether a file is an AppImage
bool isAppImage(const QString& path);

// when a file doesn't belong to the current user, this method shows a dialog asking whether to relaunch as that user
// this can be used when e.g., updating AppImages owned by root or other users
// uses pkexec, gksudo, gksu etc., whatever is available
// the second argument is the question that will be asked in the dialog displayed in case a relaunch is necessary
void checkAuthorizationAndShowDialogIfNecessary(const QString& path, const QString& question);

// searchs for path to private data directory relative to the current binary's location
// returns empty string if the path cannot be found
QString pathToPrivateDataDirectory();

// clean up desktop integration files installed while originally integrating the AppImage
bool unregisterAppImage(const QString& pathToAppImage);

// try to load icon with provided name from AppImageLauncher's fallback icons directory
// returns empty QIcon if such an icon cannot be found
// you can check for errors by calling QIcon::isNull()
QIcon loadIconWithFallback(const QString& iconName);

// sets up paths to fallback icons bundled with AppImageLauncher
void setUpFallbackIconPaths(QWidget*);
