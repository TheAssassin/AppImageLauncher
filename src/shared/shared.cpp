// system includes
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>
extern "C" {
    #include <appimage/appimage.h>
    #include <glib.h>
    // #include <libgen.h>
    #include <sys/stat.h>
    #include <stdio.h>
    #include <unistd.h>
}

// library includes
#include <QDebug>
#include <QIcon>
#include <QtDBus>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLibraryInfo>
#include <QMap>
#include <QMapIterator>
#include <QMessageBox>
#include <QObject>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>
#include <QWindow>
#include <QPushButton>
#include <QPixmap>
#ifdef ENABLE_UPDATE_HELPER
#include <appimage/update.h>
#endif

// local headers
#include "shared.h"
#include "translationmanager.h"

static void gKeyFileDeleter(GKeyFile* ptr) {
    if (ptr != nullptr)
        g_key_file_free(ptr);
}

static void gErrorDeleter(GError* ptr) {
    if (ptr != nullptr)
        g_error_free(ptr);
}

bool makeExecutable(const QString& path) {
    struct stat fileStat{};

    if (stat(path.toStdString().c_str(), &fileStat) != 0) {
        std::cerr << "Failed to call stat() on " << path.toStdString() << std::endl;
        return false;
    }

    // no action required when file is executable already
    // this could happen in scenarios when an AppImage is in a read-only location
    if ((fileStat.st_uid == getuid() && fileStat.st_mode & 0100) ||
        (fileStat.st_gid == getgid() && fileStat.st_mode & 0010) ||
        (fileStat.st_mode & 0001)) {
        return true;
    }

    return chmod(path.toStdString().c_str(), fileStat.st_mode | 0111) == 0;
}

bool makeNonExecutable(const QString& path) {
    struct stat fileStat{};

    if (stat(path.toStdString().c_str(), &fileStat) != 0) {
        std::cerr << "Failed to call stat() on " << path.toStdString() << std::endl;
        return false;
    }

    auto permissions = fileStat.st_mode;

    // remove executable permissions
    for (const auto permPart : {0100, 0010, 0001}) {
        if (permissions & permPart)
            permissions -= permPart;
    }

    return chmod(path.toStdString().c_str(), permissions) == 0;
}

QString expandTilde(QString path) {
    if ((path.size() == 1 && path[0] == '~') || (path.size() >= 2 && path.startsWith("~/"))) {
        path.remove(0, 1);
        path.prepend(QDir::homePath());
    }

    return path;
}

// calculate path to config file
QString getConfigFilePath() {
    const auto configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    const auto configFilePath = configPath + "/appimagelauncher.cfg";
    return configFilePath;
}

void createConfigFile(int askToMove,
                      const QString& destination,
                      int enableDaemon,
                      const QStringList& additionalDirsToWatch,
                      int monitorMountedFilesystems,
                      int createCliSymlinks,
                      int useSimplifiedNames) {
    auto configFilePath = getConfigFilePath();

    QFile file(configFilePath);
    file.open(QIODevice::WriteOnly);

    // cannot use QSettings because it doesn't support comments
    // let's do it manually and hope for the best
    file.write("[AppImageLauncher]\n");

    if (askToMove < 0) {
        file.write("# ask_to_move = true\n");
    } else {
        file.write("ask_to_move = ");
        if (askToMove == 0) {
            file.write("false");
        } else {
            file.write("true");
        }
        file.write("\n");
    }

    if (destination.isEmpty()) {
        file.write("# destination = ~/Applications\n");
    } else {
        file.write("destination = " + destination.toUtf8() + "\n");
    }

    if (enableDaemon < 0) {
        file.write("# enable_daemon = true\n");
    } else {
        file.write("enable_daemon = ");
        if (enableDaemon == 0) {
            file.write("false");
        } else {
            file.write("true");
        }
        file.write("\n");
    }

    if (createCliSymlinks < 0) {
        file.write("# create_cli_symlinks = true\n");
    } else {
        file.write("create_cli_symlinks = ");
        if (createCliSymlinks == 0) {
            file.write("false");
        } else {
            file.write("true");
        }
        file.write("\n");
    }

    if (useSimplifiedNames < 0) {
        file.write("# use_simplified_names = true\n");
    } else {
        file.write("use_simplified_names = ");
        if (useSimplifiedNames == 0) {
            file.write("false");
        } else {
            file.write("true");
        }
        file.write("\n");
    }

    file.write("\n\n[appimagelauncherd]\n");

    if (additionalDirsToWatch.empty()) {
        file.write("# additional_directories_to_watch = ~/otherApplications:/even/more/applications\n");
    } else {
        file.write("additional_directories_to_watch = ");
        for (int i = 0; i < additionalDirsToWatch.size(); i++) {
            file.write(additionalDirsToWatch[i].toUtf8());

            if (i < additionalDirsToWatch.size() - 1)
                file.write(":");
        }
        file.write("\n");
    }

    if (monitorMountedFilesystems < 0) {
        file.write("# monitor_mounted_filesystems = false\n");
    } else {
        file.write("monitor_mounted_filesystems = ");
        if (monitorMountedFilesystems == 0) {
            file.write("false");
        } else {
            file.write("true");
        }
        file.write("\n");
    }
}

QSettings* getConfig(QObject* parent) {
    auto configFilePath = getConfigFilePath();

    auto* settings = new QSettings(configFilePath, QSettings::IniFormat, parent);

    // expand ~ in paths in the config file with $HOME
    const auto keysContainingPath = {
        "AppImageLauncher/destination",
    };
    for (const QString& keyContainingPath : keysContainingPath){
        if (settings->contains(keyContainingPath)) {
            auto newValue = expandTilde(settings->value(keyContainingPath).toString());
            settings->setValue(keyContainingPath, newValue);
        }
    }

    return settings;
}

// TODO: check if this works with Wayland
bool isHeadless() {
    bool isHeadless = true;

    // not really clean to abuse env vars as "global storage", but hey, it works
    if (getenv("_FORCE_HEADLESS")) {
        return true;
    }

    QProcess proc;
    proc.setProgram("xhost");
    proc.setStandardOutputFile(QProcess::nullDevice());
    proc.setStandardErrorFile(QProcess::nullDevice());

    proc.start();
    proc.waitForFinished();

    switch (proc.exitCode()) {
        case 255: {
            // program not found, using fallback method
            isHeadless = (getenv("DISPLAY") == nullptr);
            break;
        }
        case 0:
        case 1:
            isHeadless = proc.exitCode() == 1;
            break;
        default:
            throw std::runtime_error("Headless detection failed: unexpected exit code from xhost");
    }

    return isHeadless;
}

// avoids code duplication, and works for both graphical and non-graphical environments
void displayMessageBox(const QString& title, const QString& message, const QMessageBox::Icon icon) {
    if (isHeadless()) {
        std::cerr << title.toStdString() << ": " << message.toStdString() << std::endl;
    } else {
        // little complex, can't use QMessageBox::{critical,warning,...} for the same reason as in main()
        auto* mb = new QMessageBox(icon, title, message, QMessageBox::Ok, nullptr);
        mb->show();
        QApplication::exec();
    }
}

void displayError(const QString& message) {
    displayMessageBox(QObject::tr("Error"), message, QMessageBox::Critical);
}

void displayWarning(const QString& message) {
    displayMessageBox(QObject::tr("Warning"), message, QMessageBox::Warning);
}

QDir integratedAppImagesDestination() {
    auto config = getConfig();

    if (config == nullptr)
        return DEFAULT_INTEGRATION_DESTINATION;

    static const QString keyName("AppImageLauncher/destination");
    if (config->contains(keyName))
        return config->value(keyName).toString();

    return DEFAULT_INTEGRATION_DESTINATION;
}

class Mount {
private:
    QString device;
    QString mountPoint;
    QString fsType;
    QString mountOptions;

public:
    Mount(QString device, QString mountPoint, QString fsType, QString mountOptions) :
        device(std::move(device)),
        mountPoint(std::move(mountPoint)),
        fsType(std::move(fsType)),
        mountOptions(std::move(
        mountOptions)) {}

    Mount(const Mount& other) = default;

    Mount& operator=(const Mount& other) = default;

public:
    const QString& getDevice() const {
        return device;
    }

    const QString& getMountPoint() const {
        return mountPoint;
    }

    const QString& getFsType() const {
        return fsType;
    }

    const QString& getMountOptions() const {
        return mountOptions;
    }
};

QList<Mount> listMounts() {
    QList<Mount> mountedDirectories;

    std::ifstream ifs("/proc/mounts");

    std::string _currentLine;
    while (std::getline(ifs, _currentLine)) {
        const auto currentLine = QString::fromStdString(_currentLine);

        const auto parts = currentLine.split(" ");

        mountedDirectories << Mount{parts[0], parts[1], parts[2], parts[3]};
    }

    return mountedDirectories;
}

QSet<QString> additionalAppImagesLocations(const bool includeAllMountPoints) {
    QSet<QString> additionalLocations;

    additionalLocations << "/Applications";

    // integrate AppImages from mounted filesystems, if requested
    // we don't want to read files from any FUSE mounted filesystems nor from any virtual filesystems
    // to
    static const auto validFilesystems = {"ext2", "ext3", "ext4", "ntfs", "vfat", "btrfs"};

    static const auto blacklistedMountPointPrefixes = {
        "/var/lib/schroot",
        "/run/docker",
        "/boot",
        "/sys",
        "/proc",
        "/snap",
    };

    if (includeAllMountPoints) {
        for (const auto& mount : listMounts()) {
            const auto& device = mount.getDevice();
            const auto& mountPoint = mount.getMountPoint();
            const auto& fsType = mount.getFsType();

            // we have to filter out virtual filesystems, i.e., ones which have a "nonsense" device path
            // any device that doesn't start with / is likely virtual, this is the first indicator
            if (device.size() < 1 || device[0] != '/') {
                continue;
            }

            // the device should exist for obvious reasons
            if (!QFileInfo(QFileInfo(device).absoluteFilePath()).exists()) {
                continue;
            }

            // we don't want to mount any loop-mounted or bind-mounted or other devices, only... "native" ones
            // therefore we permit only "real" devices listed within /dev
            if (!device.startsWith("/dev/")) {
                continue;
            }

            // there's a few locations which we know we don't want to search for AppImages in
            // either it's a waste of time or otherwise a bad idea, but it will surely save time *not* to search them
            if (std::find_if(blacklistedMountPointPrefixes.begin(), blacklistedMountPointPrefixes.end(),
                             [&mountPoint](const QString& prefix) {
                                 return mountPoint == prefix || mountPoint.startsWith(prefix + "/");
                             }) != blacklistedMountPointPrefixes.end()) {
                continue;
            }

            // we can skip the root mount point, as we handled it above
            if (mountPoint == "/") {
                continue;
            }

            // we only support a limited set of filesystems
            if (std::find(validFilesystems.begin(), validFilesystems.end(), fsType) == validFilesystems.end()) {
                continue;
            }

            // sanity check -- can likely be removed in the future
            if (mountPoint.isEmpty()) {
                const auto message = "empty mount point for mount with device " + device.toStdString();
                throw std::invalid_argument(message);
            }

            // assemble potential applications location; caller needs to check whether the directory exists before setting
            // up e.g., an inotify watch
            const QString additionalLocation(mountPoint + "/Applications");
            additionalLocations << additionalLocation;
        }
    }

    return additionalLocations;
}

bool shallMonitorMountedFilesystems(const QSettings* config) {
    Q_ASSERT(config != nullptr);

    return config->value("appimagelauncherd/monitor_mounted_filesystems", "false").toBool();
}

QDirSet getAdditionalDirectoriesFromConfig(const QSettings* config) {
    Q_ASSERT(config != nullptr);

    constexpr auto configKey = "appimagelauncherd/additional_directories_to_watch";
    const auto configValue = config->value(configKey, "").toString();
    qDebug() << configKey << "value:" << configValue;

    QDirSet additionalDirs{};

    for (auto dirPath : configValue.split(":")) {
        // empty values will, for some reason, be interpreted as "use the home directory"
        // as we don't want to accidentally monitor the home directory, we need to skip those values
        if (dirPath.isEmpty()) {
            qDebug() << "skipping empty directory path";
            continue;
        }

        // make sure to have full path
        qDebug() << "path before tilde expansion:" << dirPath;
        dirPath = expandTilde(dirPath);
        qDebug() << "path after tilde expansion:" << dirPath;

        // non-absolute paths which don't contain a tilde cannot be resolved safely, they likley depend on the cwd
        // therefore, we need to ignore those
        if (!QFileInfo(dirPath).isAbsolute()) {
            std::cerr << "Warning: path " << dirPath.toStdString() << " can not be resolved, skipping" << std::endl;
            continue;
        }

        const QDir dir(dirPath);

        if (!dir.exists()) {
            std::cerr << "Warning: could not find directory " << dirPath.toStdString() << ", skipping" << std::endl;
            continue;
        }

        additionalDirs.insert(dir);
    }

    return additionalDirs;
}

QDirSet daemonDirectoriesToWatch(const QSettings* config) {
    QDirSet watchedDirectories;

    // of course we need to watch the main integration directory
    const auto defaultDestination = integratedAppImagesDestination();

    // make sure it exists, otherwise the daemon doesn't have anything to do
    if (!defaultDestination.exists()) {
        defaultDestination.mkdir(".");
    }

    watchedDirectories.insert(defaultDestination);

    // however, there's likely additional ones to watch, like a system-wide Applications directory
    {
        bool monitorMountedFilesystems = shallMonitorMountedFilesystems(config);

        const auto additionalDirs = additionalAppImagesLocations(monitorMountedFilesystems);

        for (const auto& d : additionalDirs) {
            watchedDirectories.insert(QDir(d).absolutePath());
        }
    }

    // also, we should include additional directories from the config file
    {
        const auto configProvidedDirectories = getAdditionalDirectoriesFromConfig(config);

        std::copy(
            configProvidedDirectories.begin(), configProvidedDirectories.end(),
            std::inserter(watchedDirectories, watchedDirectories.end())
        );
    }

    return watchedDirectories;
}

QString buildPathToIntegratedAppImage(const QString& pathToAppImage) {
    // if type 2 AppImage, we can build a "content-aware" filename
    // see #7 for details
    auto digest = getAppImageDigestMd5(pathToAppImage);

    const QFileInfo appImageInfo(pathToAppImage);

    QString baseName = appImageInfo.completeBaseName();

    // if digest is available, append a separator
    if (!digest.isEmpty()) {
        const auto digestSuffix = "_" + digest;

        // check whether digest is already contained in filename
        if (!pathToAppImage.contains(digestSuffix))
            baseName += "_" + digest;
    }

    auto fileName = baseName;

    // must not use completeSuffix() in combination with completeBasename(), otherwise the final filename is composed
    // incorrectly
    if (!appImageInfo.suffix().isEmpty()) {
        fileName += "." + appImageInfo.suffix();
    }

    return integratedAppImagesDestination().path() + "/" + fileName;
}

std::map<std::string, std::string> findCollisions(const QString& currentNameEntry) {
    std::map<std::string, std::string> collisions{};

    // default locations of desktop files on systems
    const auto directories = {
        QString("/usr/share/applications/"),
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/applications/"
    };

    for (const auto& directory : directories) {
        QDirIterator iterator(directory, QDirIterator::FollowSymlinks);

        while (iterator.hasNext()) {
            const auto filename = iterator.next();

            if (!QFileInfo(filename).isFile() || !filename.endsWith(".desktop"))
                continue;

            std::shared_ptr<GKeyFile> desktopFile(g_key_file_new(), gKeyFileDeleter);
            std::shared_ptr<GError*> error(nullptr, gErrorDeleter);

            // if the key file parser can't load the file, it's most likely not a valid desktop file, so we just skip this file
            if (!g_key_file_load_from_file(desktopFile.get(), filename.toStdString().c_str(), G_KEY_FILE_KEEP_TRANSLATIONS, error.get()))
                continue;

            auto* nameEntry = g_key_file_get_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, error.get());

            // invalid desktop file, needs to be skipped
            if (nameEntry == nullptr)
                continue;

            if (QString(nameEntry).trimmed().startsWith(currentNameEntry.trimmed())) {
                collisions[filename.toStdString()] = nameEntry;
            }
        }
    }

    return collisions;
}

bool updateDesktopDatabaseAndIconCaches() {
    const auto dataLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

    const std::map<std::string, std::string> commands = {
        {"update-desktop-database", dataLocation.toStdString() + "/applications"},
        {"gtk-update-icon-cache-3.0", dataLocation.toStdString() + "/icons/hicolor/ -t"},
        {"gtk-update-icon-cache", dataLocation.toStdString() + "/icons/hicolor/ -t"},
        {"xdg-desktop-menu", "forceupdate"},
        {"update-mime-database", dataLocation.toStdString() + "/mime "},
        {"update-icon-caches", dataLocation.toStdString() + "/icons/"},
    };

    for (const auto& command : commands) {
        // only call if the command exists
        if (system(("which " + command.first + " 2>&1 1>/dev/null").c_str()) == 0) {
            // exit codes are not evaluated intentionally
            system((command.first + " " + command.second).c_str());
        }
    }

    return true;
}

std::shared_ptr<char> getOwnBinaryPath() {
    auto path = std::shared_ptr<char>(realpath("/proc/self/exe", nullptr));

    if (path == nullptr)
        throw std::runtime_error("Could not detect path to own binary; something must be horribly broken");

    return path;
}

#ifndef BUILD_LITE
QString privateLibDirPath(const QString& srcSubdirName) {
    // PRIVATE_LIBDIR will be a relative path most likely
    // therefore, we need to detect the install prefix based on our own binary path, and then calculate the path to
    // the helper tools based on that
    const QString ownBinaryDirPath = QFileInfo(getOwnBinaryPath().get()).dir().absolutePath();
    const QString installPrefixPath = QFileInfo(ownBinaryDirPath).dir().absolutePath();
    QString privateLibDirPath = installPrefixPath + "/" + PRIVATE_LIBDIR;

    // the following lines make things work during development: here, the build dir path is inserted instead, which
    // allows for testing with the latest changes
    if (!QDir(privateLibDirPath).exists()) {
        // this makes sure that when we're running from a local dev build, we end up in the right directory
        // very important when running this code from the daemon, since it's not in the same directory as the helpers
        privateLibDirPath = ownBinaryDirPath + "/../" + srcSubdirName;
    }

    // if there is no such directory like <prefix>/bin/../lib/... or the binary is not found there, there is a chance
    // the binary is just next to this one (this is the case in the update/remove helpers)
    // therefore we compare the binary directory path with PRIVATE_LIBDIR
    if (!QDir(privateLibDirPath).exists()) {
        if (privateLibDirPath.contains(PRIVATE_LIBDIR)) {
            privateLibDirPath = ownBinaryDirPath;
        }
    }

    return privateLibDirPath;
}
#endif

bool installDesktopFileAndIcons(const QString& pathToAppImage, bool resolveCollisions) {
    if (appimage_register_in_system(pathToAppImage.toStdString().c_str(), false) != 0) {
        displayError(QObject::tr("Failed to register AppImage in system via libappimage"));
        return false;
    }

    const auto* desktopFilePath = appimage_registered_desktop_file_path(pathToAppImage.toStdString().c_str(), nullptr, false);

    // sanity check -- if the file doesn't exist, the function returns NULL
    if (desktopFilePath == nullptr) {
        displayError(QObject::tr("Failed to find integrated desktop file"));
        return false;
    }

    // check that file exists
    if (!QFile(desktopFilePath).exists()) {
        displayError(QObject::tr("Couldn't find integrated AppImage's desktop file"));
        return false;
    }

    /* write AppImageLauncher specific entries to desktop file
     *
     * unfortunately, QSettings doesn't work as a desktop file reader/writer, and libqtxdg isn't really meant to be
     * used by projects via add_subdirectory/ExternalProject
     * a system dependency is not an option for this project, and we link to glib already anyway, so let's just use
     * glib, which is known to work
     */

    std::shared_ptr<GKeyFile> desktopFile(g_key_file_new(), gKeyFileDeleter);

    std::shared_ptr<GError*> error(nullptr, gErrorDeleter);

    const auto flags = GKeyFileFlags(G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS);

    auto handleError = [error, desktopFile]() {
        std::ostringstream ss;
        ss << QObject::tr("Failed to load desktop file:").toStdString() << std::endl << (*error)->message;
        displayError(QString::fromStdString(ss.str()));
    };

    if (!g_key_file_load_from_file(desktopFile.get(), desktopFilePath, flags, error.get())) {
        handleError();
        return false;
    }

    const auto* nameEntry = g_key_file_get_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, error.get());

    if (nameEntry == nullptr) {
        displayWarning(QObject::tr("AppImage has invalid desktop file"));
    }

    if (resolveCollisions) {
        // TODO: support multilingual collisions
        auto collisions = findCollisions(nameEntry);

        // make sure to remove own entry
        collisions.erase(collisions.find(desktopFilePath));

        if (!collisions.empty()) {
            // collisions are resolved like in the filesystem: a monotonically increasing number in brackets is
            // appended to the Name in order to keep the number monotonically increasing, we look for the highest
            // number in brackets in the existing entries, add 1 to it, and append it in brackets to the current
            // desktop file's Name entry

            unsigned int currentNumber = 1;

            QRegularExpression regex(R"(^.*\(([0-9]+)\)$)");

            for (const auto& collision : collisions) {
                const auto& currentNameEntry = collision.second;

                auto match = regex.match(QString::fromStdString(currentNameEntry));

                if (match.hasMatch()) {
                    // 0 = entire string
                    // 1 = first group
                    const QString numString = match.captured(1);
                    const int num = numString.toInt();

                    // monotonic counting, i.e., never try to "be smart" by e.g., filling in the gaps between
                    // previous numbers
                    if (num >= currentNumber) {
                        currentNumber = num + 1;
                    }
                }
            }

            auto newName = QString(nameEntry) + " (" + QString::number(currentNumber) + ")";
            g_key_file_set_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, newName.toStdString().c_str());
        }
    }

    auto convertToCharPointerList = [](const std::vector<std::string>& stringList) {
        std::vector<const char*> pointerList;

        // reserve space to increase efficiency
        pointerList.reserve(stringList.size());

        // convert string list to list of const char pointers
        for (const auto& action : stringList) {
            pointerList.push_back(action.c_str());
        }

        return pointerList;
    };

    std::vector<std::string> desktopActions;

    // we may not just overwrite the existing actions key, as then the actions cannot be used any more from the context menu
    {
        const auto* actionsEntry = g_key_file_get_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ACTIONS, error.get());
        for (const QString& action : QString(actionsEntry).split(";")) {
            if (action.isEmpty()) {
                continue;
            }

            desktopActions.emplace_back(action.toStdString());
        }
    }

    // use a "vendor prefix" to avoid collisions with existing actions, as "Update" and "Remove" are generic terms
    static const std::string removeActionKey{"AppImageLauncher-Remove-AppImage"};
    static const std::string updateActionKey{"AppImageLauncher-Update-AppImage"};

    desktopActions.emplace_back(removeActionKey);

    // load translations from JSON file(s)
    QMap<QString, QString> removeActionNameTranslations;

#ifdef ENABLE_UPDATE_HELPER
    QMap<QString, QString> updateActionNameTranslations;

    {
        QDirIterator i18nDirIterator(TranslationManager::getTranslationDir());

        while(i18nDirIterator.hasNext()) {
            const auto& filePath = i18nDirIterator.next();
            const auto& fileName = QFileInfo(filePath).fileName();

            if (!QFileInfo(filePath).isFile() || !(fileName.startsWith("desktopfiles.") && fileName.endsWith(".json")))
                continue;

            // check whether filename's format is alright, otherwise parsing the locale might try to access a
            // non-existing (or the wrong) member
            auto splitFilename = fileName.split(".");

            if (splitFilename.size() != 3)
                continue;

            // parse locale from filename
            auto locale = splitFilename[1];

            QFile jsonFile(filePath);

            if (!jsonFile.open(QIODevice::ReadOnly)) {
                displayWarning(QMessageBox::tr("Could not parse desktop file translations:\nCould not open file for reading:\n\n%1").arg(fileName));
            }

            // TODO: need to make sure that this doesn't try to read huge files at once
            auto data = jsonFile.readAll();

            QJsonParseError parseError{};
            auto jsonDoc = QJsonDocument::fromJson(data, &parseError);

            // show warning on syntax errors and continue
            if (parseError.error != QJsonParseError::NoError || jsonDoc.isNull() || !jsonDoc.isObject()) {
                displayWarning(QMessageBox::tr("Could not parse desktop file translations:\nInvalid syntax:\n\n%1").arg(parseError.errorString()));
            }

            auto jsonObj = jsonDoc.object();

            for (const auto& key : jsonObj.keys()) {
                auto value = jsonObj[key].toString();
                auto splitKey = key.split("/");

                if (key.startsWith("Desktop Action update")) {
                    qDebug() << "update: adding" << value << "for locale" << locale;
                    updateActionNameTranslations[locale] = value;
                } else if (key.startsWith("Desktop Action remove")) {
                    qDebug() << "remove: adding" << value << "for locale" << locale;
                    removeActionNameTranslations[locale] = value;
                }
            }
        }
    }
#endif

#ifndef BUILD_LITE
    auto privateLibDir = privateLibDirPath("ui");

    const char helperIconName[] = "AppImageLauncher";
#else
    const char helperIconName[] = "AppImageLauncher-Lite";
#endif

    // add Remove action
    {
        const auto removeSectionName = "Desktop Action " + removeActionKey;

        g_key_file_set_string(desktopFile.get(), removeSectionName.c_str(), "Name", "Delete this AppImage");
        g_key_file_set_string(desktopFile.get(), removeSectionName.c_str(), "Icon", helperIconName);

        std::ostringstream removeExecPath;

#ifndef BUILD_LITE
        removeExecPath << privateLibDir.toStdString() << "/remove";
#else
        removeExecPath << getenv("HOME") << "/.local/lib/appimagelauncher-lite/appimagelauncher-lite.AppImage remove";
#endif

        removeExecPath << " \"" << pathToAppImage.toStdString() << "\"";

        g_key_file_set_string(desktopFile.get(), removeSectionName.c_str(), "Exec", removeExecPath.str().c_str());

        // install translations
        auto it = QMapIterator<QString, QString>(removeActionNameTranslations);
        while (it.hasNext()) {
            auto entry = it.next();
            g_key_file_set_locale_string(desktopFile.get(), removeSectionName.c_str(), "Name", entry.key().toStdString().c_str(), entry.value().toStdString().c_str());
        }
    }

#ifdef ENABLE_UPDATE_HELPER
    // add Update action
    {
        appimage::update::Updater updater(pathToAppImage.toStdString());

        // but only if there's update information
        if (!updater.updateInformation().empty()) {
            // section needs to be announced in desktop actions list
            desktopActions.emplace_back(updateActionKey);

            const auto updateSectionName = "Desktop Action " + updateActionKey;

            g_key_file_set_string(desktopFile.get(), updateSectionName.c_str(), "Name", "Update this AppImage");
            g_key_file_set_string(desktopFile.get(), updateSectionName.c_str(), "Icon", helperIconName);

            std::ostringstream updateExecPath;

#ifndef BUILD_LITE
            updateExecPath << privateLibDir.toStdString() << "/update";
#else
            updateExecPath << getenv("HOME") << "/.local/lib/appimagelauncher-lite/appimagelauncher-lite.AppImage update";
#endif
            updateExecPath << " \"" << pathToAppImage.toStdString() << "\"";

            g_key_file_set_string(desktopFile.get(), updateSectionName.c_str(), "Exec", updateExecPath.str().c_str());

            // install translations
            auto it = QMapIterator<QString, QString>(updateActionNameTranslations);
            while (it.hasNext()) {
                auto entry = it.next();
                g_key_file_set_locale_string(desktopFile.get(), updateSectionName.c_str(), "Name", entry.key().toStdString().c_str(), entry.value().toStdString().c_str());
            }
        }
    }
#endif

    // add desktop actions key
    g_key_file_set_string_list(
            desktopFile.get(),
            G_KEY_FILE_DESKTOP_GROUP,
            G_KEY_FILE_DESKTOP_KEY_ACTIONS,
            convertToCharPointerList(desktopActions).data(),
            desktopActions.size()
    );

    // add version key
    const auto version = QApplication::applicationVersion().replace("version ", "").toStdString();
    g_key_file_set_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, "X-AppImageLauncher-Version", version.c_str());

    // save desktop file to disk
    if (!g_key_file_save_to_file(desktopFile.get(), desktopFilePath, error.get())) {
        handleError();
        return false;
    }

    // make desktop file executable ("trustworthy" to some DEs)
    // TODO: handle this in libappimage
    makeExecutable(desktopFilePath);

    // notify KDE/Plasma about icon change
    {
        auto message = QDBusMessage::createSignal(QStringLiteral("/KIconLoader"), QStringLiteral("org.kde.KIconLoader"), QStringLiteral("iconChanged"));
        message.setArguments({0});
        QDBusConnection::sessionBus().send(message);
    }

    // Vérifier si l'option est activée (par défaut activée)
    auto config = getConfig();
    if (config == nullptr || !config->contains("AppImageLauncher/create_cli_symlinks") ||
        config->value("AppImageLauncher/create_cli_symlinks").toBool()) {
        
        // Créer le lien symbolique normal basé sur le hash
        QString pathToLocalBin = QDir::homePath() + "/.local/bin/";
        QDir localBinDir(pathToLocalBin);
        
        // Créer le répertoire s'il n'existe pas
        if (!localBinDir.exists()) {
            QDir().mkpath(pathToLocalBin);
        }
        
        // Récupérer ou créer un identifiant unique pour le lien (basé sur le hash MD5 de l'AppImage)
        std::string pathStr = pathToAppImage.toStdString();
        unsigned char* digest = (unsigned char*)appimage_get_md5(pathStr.c_str());
        char* md5str = appimage_hexlify((const char*)digest, 16);
        QString baseName = QFileInfo(pathToAppImage).baseName().replace(" ", "_");
        QString linkName = baseName + "_" + QString(md5str);
        free(md5str);
        free(digest);
        
        // Créer le lien symbolique
        QString targetPath = localBinDir.filePath(linkName);
        if (QFile::exists(targetPath)) {
            QFile::remove(targetPath);
        }
        QFile::link(pathToAppImage, targetPath);
        
        // Vérifier si l'option des noms simplifiés est activée
        if (config == nullptr || !config->contains("AppImageLauncher/use_simplified_names") ||
            config->value("AppImageLauncher/use_simplified_names").toBool()) {
            
            // Extraire un nom de commande adapté
            QString commandName = getCommandNameMapping(pathToAppImage);
            
            // Si aucun mappage n'existe encore, en créer un nouveau
            if (commandName.isEmpty()) {
                commandName = extractCommandName(pathToAppImage);
                
                // Vérifier s'il y a un conflit de noms
                QString simplifiedLinkPath = pathToLocalBin + commandName;
                
                if (QFile::exists(simplifiedLinkPath)) {
                    // En cas de conflit, ajouter un suffixe numérique
                    int suffix = 1;
                    while (QFile::exists(simplifiedLinkPath + "-" + QString::number(suffix))) {
                        suffix++;
                    }
                    commandName = commandName + "-" + QString::number(suffix);
                    simplifiedLinkPath = pathToLocalBin + commandName;
                }
                
                // Enregistrer le mappage pour une utilisation ultérieure
                registerCommandNameMapping(pathToAppImage, commandName);
            }
            
            // Créer le lien symbolique simplifié
            QString simplifiedLinkPath = pathToLocalBin + commandName;
            if (QFile::exists(simplifiedLinkPath)) {
                QFile::remove(simplifiedLinkPath);
            }
            QFile::link(pathToAppImage, simplifiedLinkPath);
        }
    }

    return true;
}

bool updateDesktopFileAndIcons(const QString& pathToAppImage) {
    return installDesktopFileAndIcons(pathToAppImage, true);
}

IntegrationState integrateAppImage(const QString& pathToAppImage, const QString& pathToIntegratedAppImage) {
    // need std::strings to get working pointers with .c_str()
    const auto oldPath = pathToAppImage.toStdString();
    const auto newPath = pathToIntegratedAppImage.toStdString();

    // create target directory
    QDir().mkdir(QFileInfo(QFile(pathToIntegratedAppImage)).dir().absolutePath());

    // check whether AppImage is in integration directory already
    if (QFileInfo(pathToAppImage).absoluteFilePath() != QFileInfo(pathToIntegratedAppImage).absoluteFilePath()) {
        // need to check whether file exists
        // if it does, the existing AppImage needs to be removed before rename can be called
        if (QFile(pathToIntegratedAppImage).exists()) {
            std::ostringstream message;
            message << QObject::tr("AppImage with same filename has already been integrated.").toStdString() << std::endl
                    << std::endl
                    << QObject::tr("Do you wish to overwrite the existing AppImage?").toStdString() << std::endl
                    << QObject::tr("Choosing No will run the AppImage once, and leave the system in its current state.").toStdString();

            auto* messageBox = new QMessageBox(
                QMessageBox::Warning,
                QObject::tr("Warning"),
                QString::fromStdString(message.str()),
                QMessageBox::Yes | QMessageBox::No
            );

            messageBox->setDefaultButton(QMessageBox::No);
            messageBox->show();

            QApplication::exec();

            if (messageBox->clickedButton() == messageBox->button(QMessageBox::No)) {
                return INTEGRATION_ABORTED;
            }

            QFile(pathToIntegratedAppImage).remove();
        }

        if (!QFile(pathToAppImage).rename(pathToIntegratedAppImage)) {
            auto* messageBox = new QMessageBox(
                QMessageBox::Critical,
                QObject::tr("Error"),
                QObject::tr("Failed to move AppImage to target location.\n"
                            "Try to copy AppImage instead?"),
                QMessageBox::Ok | QMessageBox::Cancel
            );

            messageBox->setDefaultButton(QMessageBox::Ok);
            messageBox->show();

            QApplication::exec();

            if (messageBox->clickedButton() == messageBox->button(QMessageBox::Cancel))
                return INTEGRATION_FAILED;

            if (!QFile(pathToAppImage).copy(pathToIntegratedAppImage)) {
                displayError("Failed to copy AppImage to target location");
                return INTEGRATION_FAILED;
            }
        }
    }

    if (!installDesktopFileAndIcons(pathToIntegratedAppImage))
        return INTEGRATION_FAILED;

    return INTEGRATION_SUCCESSFUL;
}

QString getAppImageDigestMd5(const QString& path) {
    // try to read embedded MD5 digest
    unsigned long offset = 0, length = 0;

    // first of all, digest calculation is supported only for type 2
    if (appimage_get_type(path.toStdString().c_str(), false) != 2)
        return "";

    auto rv = appimage_get_elf_section_offset_and_length(path.toStdString().c_str(), ".digest_md5", &offset, &length);

    QByteArray buffer(16, '\0');

    if (rv && offset != 0 && length != 0) {
        // open file and read digest from ELF header section
        QFile file(path);

        if (!file.open(QFile::ReadOnly))
            return "";

        if (!file.seek(static_cast<qint64>(offset)))
            return "";

        if (!file.read(buffer.data(), buffer.size()))
            return "";

        file.close();
    }

    bool needToCalculateDigest;

    // there seem to be some AppImages out there who actually have the required section embedded, but it's empty
    // therefore we make the assumption that a hash value of zeroes is probably incorrect and recalculate
    // in the extremely rare case in which the AppImage's digest would *really* be that value, we'd waste a bit of
    // computation time, but the chances are so low... who cares, right?
    {
        auto nonZeroCharacterFound = false;

        for (const char i : buffer) {
            if (i != '\0') {
                nonZeroCharacterFound = true;
                break;
            }
        }

        needToCalculateDigest = !nonZeroCharacterFound;
    }

    if (needToCalculateDigest) {
        // calculate digest
        if (!appimage_type2_digest_md5(path.toStdString().c_str(), buffer.data()))
            return "";
    }

    // create hexadecimal representation
    auto hexDigest = appimage_hexlify(buffer, static_cast<size_t>(buffer.size()));

    QString hexDigestStr(hexDigest);

    free(hexDigest);

    return hexDigestStr;
}

bool hasAlreadyBeenIntegrated(const QString& pathToAppImage) {
    return appimage_is_registered_in_system(pathToAppImage.toStdString().c_str());
}

bool isInDirectory(const QString& pathToAppImage, const QDir& directory) {
    return directory == QFileInfo(pathToAppImage).absoluteDir();
}

bool cleanUpOldDesktopIntegrationResources(bool verbose) {
    auto dirPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/applications";

    auto directory = QDir(dirPath);

    QStringList filters;
    filters << "appimagekit_*.desktop";

    directory.setNameFilters(filters);

    for (auto desktopFilePath : directory.entryList()) {
        desktopFilePath = dirPath + "/" + desktopFilePath;

        std::shared_ptr<GKeyFile> desktopFile(g_key_file_new(), [](GKeyFile* p) {
            g_key_file_free(p);
        });

        if (!g_key_file_load_from_file(desktopFile.get(), desktopFilePath.toStdString().c_str(), G_KEY_FILE_NONE, nullptr)) {
            continue;
        }

        std::shared_ptr<char> execValue(g_key_file_get_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC, nullptr), [](char* p) {
            free(p);
        });

        // if there is no Exec value in the file, the desktop file is apparently broken, therefore we skip the file
        if (execValue == nullptr) {
            continue;
        }

        std::shared_ptr<char> tryExecValue(g_key_file_get_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TRY_EXEC, nullptr), [](char* p) {
            free(p);
        });

        // TryExec is optional, although recently the desktop integration functions started to force add such keys
        // with a path to the desktop file
        // (before, if it existed, the key was replaced with the AppImage's path)
        // If it exists, we assume its value is the full path to the AppImage, which can be used to check the existence
        // of the AppImage
        QString appImagePath;

        if (tryExecValue != nullptr) {
            appImagePath = QString(tryExecValue.get());
        } else {
            appImagePath = QString(execValue.get()).split(" ").first();
        }

        // now, check whether AppImage exists
        // FIXME: the split command for the Exec value might not work if there's a space in the filename
        // we really need a parser that understands the desktop file escaping
        if (!QFile(appImagePath).exists()) {
            if (verbose)
                std::cout << "AppImage no longer exists, cleaning up resources: " << appImagePath.toStdString() << std::endl;

            if (verbose)
                std::cout << "Removing desktop file: " << desktopFilePath.toStdString() << std::endl;

            QFile(desktopFilePath).remove();

            // TODO: clean up related resources such as icons or MIME definitions

            auto* iconValue = g_key_file_get_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, nullptr);

            if (iconValue != nullptr) {
                const auto dataLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
                const auto iconsPath = QString::fromStdString(dataLocation.toStdString() + "/share/icons/");

                for (QDirIterator it(iconsPath, QDirIterator::Subdirectories); it.hasNext();) {
                    auto path = it.next();

                    if (QFileInfo(path).completeBaseName().startsWith(iconValue)) {
                        QFile::remove(path);
                    }
                }
            }
        }
    }

    return true;
}

time_t getMTime(const QString& path) {
    struct stat st{};
    if (stat(path.toStdString().c_str(), &st) != 0) {
        displayError(QObject::tr("Failed to call stat() on path:\n\n%1").arg(path));
        return -1;
    }

    return st.st_mtim.tv_sec;
}

bool desktopFileHasBeenUpdatedSinceLastUpdate(const QString& pathToAppImage) {
    const auto ownBinaryPath = getOwnBinaryPath();

    const auto desktopFilePath = appimage_registered_desktop_file_path(pathToAppImage.toStdString().c_str(), nullptr, false);
    
    auto ownBinaryMTime = getMTime(ownBinaryPath.get());
    auto desktopFileMTime = getMTime(desktopFilePath);

    // check if something has failed horribly
    if (desktopFileMTime < 0 || ownBinaryMTime < 0)
        return false;

    return desktopFileMTime > ownBinaryMTime;
}

bool isAppImage(const QString& path) {
    const auto type = appimage_get_type(path.toUtf8(), false);
    return type > 0 && type <= 2;
}

QString which(const std::string& name) {
    std::vector<char> command(4096);
    snprintf(command.data(), command.size()-1, "which %s", name.c_str());

    auto* proc = popen(command.data(), "r");

    if (proc == nullptr)
        throw std::runtime_error("Failed to start process for which");

    std::vector<char> outBuf(4096);

    fread(outBuf.data(), sizeof(char), outBuf.size()-1, proc);

    pclose(proc);

    QString rv(outBuf.data());

    rv.replace("\n", "");

    return rv;
}

void checkAuthorizationAndShowDialogIfNecessary(const QString& path, const QString& question) {
    const uint32_t ownUid = getuid();
    const uint32_t fileOwnerUid = QFileInfo(path).ownerId();
    const auto fileOwnerUsername = QFileInfo(path).owner();

    if (ownUid != fileOwnerUid) {
        qDebug() << "attempting relaunch with root helper";

        QString messageBoxText = QMessageBox::tr("File %1 is owned by another user: %2").arg(path).arg(fileOwnerUsername);
        messageBoxText += "\n\n";
        messageBoxText += question;

        auto* messageBox = new QMessageBox(
            QMessageBox::Warning,
            QMessageBox::tr("Permissions problem"),
            messageBoxText,
            QMessageBox::Ok | QMessageBox::Abort,
            nullptr
        );

        messageBox->setDefaultButton(QMessageBox::Ok);
        messageBox->show();

        QApplication::exec();

        const auto relaunch = messageBox->clickedButton() == messageBox->button(QMessageBox::Ok);

        if (!relaunch) {
            qDebug() << "Dialog aborted";
            exit(1);
        }

        qDebug() << "ok, attempting relaunch with root helper";

        // pkexec doesn't retain $DISPLAY etc., as per the man page, so we can't run UI programs with it
        for (const auto& rootHelperFilename : {/*"pkexec",*/ "gksudo", "gksu"}) {
            const auto rootHelperPath = which(rootHelperFilename);
            qDebug() << "trying root helper " << rootHelperFilename << rootHelperPath;

            if (rootHelperPath.isEmpty())
                continue;

            qDebug() << rootHelperFilename << rootHelperPath;

            std::vector<char*> argv = {
                strdup(rootHelperPath.toStdString().c_str()),
            };

            if (fileOwnerUid != 0) {
                argv.emplace_back(strdup("--user"));
                argv.emplace_back(strdup(std::to_string(fileOwnerUid).c_str()));
            }

            for (const auto& arg : QCoreApplication::arguments()) {
                argv.emplace_back(strdup(arg.toStdString().c_str()));
            }

            argv.emplace_back(nullptr);

            const auto rv = execv(strdup(rootHelperPath.toStdString().c_str()), argv.data());

            // if the execution fails, we should signalize this to the user instead of silently failing over to the
            // next tool
            QMessageBox::critical(
                    nullptr,
                    QMessageBox::tr("Error"),
                    QMessageBox::tr("Failed to run permissions helper, exited with return code %1").arg(rv)
            );
            exit(1);
        }

        QMessageBox::critical(
            nullptr,
            QMessageBox::tr("Error"),
            QMessageBox::tr("Could not find suitable permissions helper, aborting")
        );
        exit(1);
    }
}

QString pathToPrivateDataDirectory() {
    // first we need to find the translation directory
    // if this is run from the build tree, we try a path that can only work within the build directory
    // then, we try the expected install location relative to the main binary
    const auto binaryDirPath = QApplication::applicationDirPath();

    // our helper tools are not shipped in usr/bin but usr/lib/<arch>-linux-gnu/appimagelauncher
    // therefore we need to check for the translations directory relative to this directory as well
    // as <arch-linux-gnu> may not be used in the path, we also check for its parent directory
    QString dataDir = binaryDirPath + "/../../share/appimagelauncher/";

    if (!QDir(dataDir).exists()) {
        dataDir = binaryDirPath + "/../../../share/appimagelauncher/";
    }

    // this directory should work for the main application in usr/bin
    if (!QDir(dataDir).exists()) {
        dataDir = binaryDirPath + "/../share/appimagelauncher/";
    }

    if (!QDir(dataDir).exists()) {
        std::cerr << "[AppImageLauncher] Warning: "
                  << "Path to private data directory could not be found" << std::endl;
        return "";
    }

    return dataDir;
}

bool unregisterAppImage(const QString& pathToAppImage) {
    auto rv = appimage_unregister_in_system(pathToAppImage.toStdString().c_str(), false);
    
    if (rv != 0)
        return false;
    
    // Supprimer le lien symbolique normal
    removeSymlinkFromPath(pathToAppImage);
    
    // Supprimer le lien symbolique simplifié
    QString commandName = getCommandNameMapping(pathToAppImage);
    if (!commandName.isEmpty()) {
        QString simplifiedLinkPath = QDir::homePath() + "/.local/bin/" + commandName;
        if (QFile::exists(simplifiedLinkPath)) {
            QFile::remove(simplifiedLinkPath);
        }
        removeCommandNameMapping(pathToAppImage);
    }
    
    return true;
}

QIcon loadIconWithFallback(const QString& iconName) {
    const QString subdirName("fallback-icons");
    const auto binaryDir = QApplication::applicationDirPath();

    // first we check the directory that would be expected with in the build environment
    QDir fallbackIconDirectory = QDir(binaryDir + "/../../resources/" + subdirName);

    // if that doesn't work, we check the private data directory, which should work when AppImageLauncher is installed
    // through the packages or in Lite's AppImage
    if (!fallbackIconDirectory.exists()) {
        auto privateDataDir = pathToPrivateDataDirectory();

        if (privateDataDir.length() > 0 && QDir(privateDataDir).exists()) {
            fallbackIconDirectory = QDir(pathToPrivateDataDirectory() + "/" + subdirName);
        }
    }

    // fallback icons aren't critical enough to exit the application if they can't be found
    // after all, the theme icons may work just as well
    if (!fallbackIconDirectory.exists()) {
        std::cerr << "[AppImageLauncher] Warning:"
                  << "fallback icons could not be loaded: directory could not be found" << std::endl;
        return QIcon{};
    }

    qDebug() << "Loading fallback for icon" << iconName;

    const auto iconFilename = iconName + ".svg";
    const auto iconPath = fallbackIconDirectory.filePath(iconFilename);

    if (!QFileInfo(iconPath).isFile()) {
        std::cerr << "[AppImageLauncher] Warning: can't find fallback icon for name"
                  << iconName.toStdString() << std::endl;
        return QIcon{};
    }

    const auto fallbackIcon = QIcon(iconPath);
    qDebug() << fallbackIcon;

    return fallbackIcon;
}

void setUpFallbackIconPaths(QWidget* parent) {
    /**
     * Qt 5.12 adds a feature to add fallback paths for icons. This is a very simple way to automatically load custom
     * icons when the icon theme doesn't provide a suitable alternative.
     * However, we need to support a much older Qt version. Therefore we cannot use this very very handy feature.
     * We basically iterate over all buttons which carry an icon and (re)load it, but this time provide a fallback
     * loaded from our private data directory.
     */

    // for now we only support buttons
    // we could always add more widgets which provide an icon property
    const auto buttons = parent->findChildren<QAbstractButton*>();

    for (const auto& button : buttons) {
        const auto iconName = button->icon().name();

        // sort out buttons without an icon
        if (iconName.length() <= 0)
            continue;

        // load icon from theme, providing the bundled icon as a fallback
        // loading an "empty" (i.e., isNull() returns true) icon as fallback, as returned by loadIconWithFallback(...),
        // works just fine
        auto fallbackIcon = loadIconWithFallback(iconName);
        auto newIcon = QIcon::fromTheme(iconName, fallbackIcon);

        if (newIcon.isNull() || newIcon.pixmap(16, 16).isNull())
            newIcon = fallbackIcon;

        // now replace the button's actual icon with the fallback-enabled one
        button->setIcon(newIcon);
    }
}

bool isLocalBinInPath() {
    const QString localBinPath = QDir::homePath() + "/.local/bin";
    
    // Récupérer le PATH depuis les variables d'environnement
    const QString pathEnv = qEnvironmentVariable("PATH");
    const QStringList pathDirs = pathEnv.split(":", Qt::SkipEmptyParts);
    
    // Vérifier si le répertoire est dans le PATH
    return pathDirs.contains(localBinPath);
}

bool createSymlinkInPath(const QString& pathToAppImage) {
    // Obtenir le nom de fichier de l'AppImage
    QFileInfo appImageInfo(pathToAppImage);
    
    // Utiliser le basename sans extension pour le lien symbolique
    QString baseName = appImageInfo.completeBaseName();
    
    // Sanitiser le nom (remplacer les espaces et autres caractères problématiques)
    baseName.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");
    
    // Vérifier s'il existe déjà une AppImage avec le même basename
    // Si c'est le cas, ajouter un suffixe (un hash md5 par exemple)
    QString linkName = baseName;
    
    QDir localBinDir(QDir::homePath() + "/.local/bin");
    
    // Créer le répertoire ~/.local/bin s'il n'existe pas
    if (!localBinDir.exists()) {
        if (!localBinDir.mkpath(".")) {
            std::cerr << "Failed to create directory " << localBinDir.path().toStdString() << std::endl;
            return false;
        }
    }
    
    // Si un lien portant ce nom existe déjà mais pointe vers un fichier différent
    QString existingLinkPath = localBinDir.filePath(linkName);
    if (QFile::exists(existingLinkPath)) {
        QFileInfo existingLinkInfo(existingLinkPath);
        
        // Si c'est un lien symbolique qui pointe vers un autre fichier
        if (existingLinkInfo.isSymLink() && 
            QFileInfo(existingLinkInfo.symLinkTarget()).canonicalFilePath() != 
            QFileInfo(pathToAppImage).canonicalFilePath()) {
            
            // Ajouter un suffixe basé sur un hash md5 pour éviter les conflits
            QString digest = getAppImageDigestMd5(pathToAppImage);
            if (!digest.isEmpty()) {
                linkName = baseName + "_" + digest.left(8);
            } else {
                // Si pas de digest disponible, utiliser un timestamp
                linkName = baseName + "_" + QString::number(QDateTime::currentSecsSinceEpoch());
            }
        } else if (!existingLinkInfo.isSymLink()) {
            // Si c'est un fichier normal et non un lien, ne pas l'écraser
            std::cerr << "A file with name " << linkName.toStdString() 
                      << " already exists in " << localBinDir.path().toStdString() 
                      << " and is not a symlink" << std::endl;
            return false;
        }
    }
    
    // Créer le lien symbolique
    QString targetPath = localBinDir.filePath(linkName);
    
    // Supprimer le lien existant s'il pointe vers la même AppImage
    if (QFile::exists(targetPath)) {
        if (!QFile::remove(targetPath)) {
            std::cerr << "Failed to remove existing symlink " << targetPath.toStdString() << std::endl;
            return false;
        }
    }
    
    if (!QFile::link(appImageInfo.absoluteFilePath(), targetPath)) {
        std::cerr << "Failed to create symlink from " << appImageInfo.absoluteFilePath().toStdString() 
                  << " to " << targetPath.toStdString() << std::endl;
        return false;
    }
    
    // Afficher un avertissement si ~/.local/bin n'est pas dans le PATH
    if (!isLocalBinInPath()) {
        std::cerr << "Warning: ~/.local/bin is not in PATH. " 
                  << "You may need to add it to your PATH to use the command-line shortcut." << std::endl;
    }
    
    return true;
}

bool removeSymlinkFromPath(const QString& pathToAppImage) {
    // Obtenir le nom de fichier de l'AppImage
    QFileInfo appImageInfo(pathToAppImage);
    
    // Utiliser le basename sans extension pour le lien symbolique
    QString baseName = appImageInfo.completeBaseName();
    
    // Sanitiser le nom comme lors de la création
    baseName.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");
    
    QDir localBinDir(QDir::homePath() + "/.local/bin");
    if (!localBinDir.exists()) {
        // Rien à supprimer si le répertoire n'existe pas
        return true;
    }
    
    // Vérifier si le lien standard existe
    QString standardLinkPath = localBinDir.filePath(baseName);
    if (QFile::exists(standardLinkPath)) {
        QFileInfo linkInfo(standardLinkPath);
        
        // Vérifier si c'est un lien symbolique qui pointe vers notre AppImage
        if (linkInfo.isSymLink() &&
            QFileInfo(linkInfo.symLinkTarget()).canonicalFilePath() == 
            QFileInfo(pathToAppImage).canonicalFilePath()) {
            
            if (!QFile::remove(standardLinkPath)) {
                std::cerr << "Failed to remove symlink " << standardLinkPath.toStdString() << std::endl;
                return false;
            }
            return true;
        }
    }
    
    // Si le lien standard n'existait pas ou ne pointait pas vers notre AppImage,
    // chercher un lien avec suffixe MD5
    QString digest = getAppImageDigestMd5(pathToAppImage);
    if (!digest.isEmpty()) {
        QString hashLinkPath = localBinDir.filePath(baseName + "_" + digest.left(8));
        
        if (QFile::exists(hashLinkPath)) {
            QFileInfo linkInfo(hashLinkPath);
            
            if (linkInfo.isSymLink() &&
                QFileInfo(linkInfo.symLinkTarget()).canonicalFilePath() == 
                QFileInfo(pathToAppImage).canonicalFilePath()) {
                
                if (!QFile::remove(hashLinkPath)) {
                    std::cerr << "Failed to remove symlink " << hashLinkPath.toStdString() << std::endl;
                    return false;
                }
                return true;
            }
        }
    }
    
    // Aucun lien trouvé pointant vers cette AppImage
    return true;
}

bool synchronizeSymlinksForIntegratedAppImages() {
    bool success = true;
    QDir appsDir = integratedAppImagesDestination();
    
    // Vérifier si l'option est activée
    auto config = getConfig();
    const bool createSymlinks = config == nullptr || !config->contains("AppImageLauncher/create_cli_symlinks") ||
                               config->value("AppImageLauncher/create_cli_symlinks").toBool();
    
    if (!createSymlinks) {
        // Si l'option est désactivée, supprimer tous les liens existants
        QDir localBinDir(QDir::homePath() + "/.local/bin");
        if (!localBinDir.exists()) {
            return true; // Rien à faire si le répertoire n'existe pas
        }
        
        QStringList entries = localBinDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QString& entry : entries) {
            QString linkPath = localBinDir.absoluteFilePath(entry);
            
            if (QFileInfo(linkPath).isSymLink()) {
                QString target = QFile::symLinkTarget(linkPath);
                
                // Vérifier si la cible est dans le répertoire des AppImages intégrées
                if (target.startsWith(appsDir.absolutePath())) {
                    QFile::remove(linkPath);
                }
            }
        }
        
        // Effacer tous les mappages de noms de commande
        QSettings mappings(getCommandNameMappingFilePath(), QSettings::IniFormat);
        mappings.clear();
        
        return true;
    }
    
    // Si l'option est activée, synchroniser les liens
    QStringList appImages = appsDir.entryList(QStringList() << "*.AppImage", QDir::Files);
    
    // Vérifier si le répertoire ~/.local/bin existe, sinon le créer
    QString localBinPath = QDir::homePath() + "/.local/bin";
    QDir localBinDir(localBinPath);
    if (!localBinDir.exists()) {
        QDir().mkpath(localBinPath);
    }
    
    // Vérifier si l'option des noms simplifiés est activée
    const bool useSimplifiedNames = config == nullptr || !config->contains("AppImageLauncher/use_simplified_names") ||
                                config->value("AppImageLauncher/use_simplified_names").toBool();
    
    // Collecter les noms existants pour détecter les conflits
    QStringList existingNames;
    
    for (const QString& appImageName : appImages) {
        QString appImagePath = appsDir.absoluteFilePath(appImageName);
        
        // Créer le lien symbolique normal
        std::string pathStr = appImagePath.toStdString();
        unsigned char* digest = (unsigned char*)appimage_get_md5(pathStr.c_str());
        char* md5str = appimage_hexlify((const char*)digest, 16);
        QString baseName = QFileInfo(appImagePath).baseName().replace(" ", "_");
        QString linkName = baseName + "_" + QString(md5str);
        free(md5str);
        free(digest);
        
        QString linkPath = localBinPath + "/" + linkName;
        if (QFile::exists(linkPath)) {
            QFile::remove(linkPath);
        }
        QFile::link(appImagePath, linkPath);
        
        // Si l'option des noms simplifiés est activée, gérer également ces liens
        if (useSimplifiedNames) {
            // Vérifier si un mappage existe déjà
            QString commandName = getCommandNameMapping(appImagePath);
            
            // Si pas de mappage, en créer un nouveau
            if (commandName.isEmpty()) {
                commandName = extractCommandName(appImagePath);
                
                // Gérer les conflits
                QString baseName = commandName;
                int suffix = 1;
                while (existingNames.contains(commandName)) {
                    commandName = baseName + "-" + QString::number(suffix);
                    suffix++;
                }
                
                registerCommandNameMapping(appImagePath, commandName);
            }
            
            existingNames.append(commandName);
            
            // Créer ou mettre à jour le lien symbolique simplifié
            QString simplifiedLinkPath = localBinPath + "/" + commandName;
            if (QFile::exists(simplifiedLinkPath)) {
                QFile::remove(simplifiedLinkPath);
            }
            QFile::link(appImagePath, simplifiedLinkPath);
        } else {
            // Si les noms simplifiés sont désactivés, supprimer les liens et mappages existants
            QString commandName = getCommandNameMapping(appImagePath);
            if (!commandName.isEmpty()) {
                QString simplifiedLinkPath = localBinPath + "/" + commandName;
                if (QFile::exists(simplifiedLinkPath)) {
                    QFile::remove(simplifiedLinkPath);
                }
                removeCommandNameMapping(appImagePath);
            }
        }
    }
    
    return success;
}

QString sanitizeCommandName(const QString& name) {
    QString result = name.toLower();
    // Remplacer espaces et caractères spéciaux
    result.replace(QRegExp("[^a-zA-Z0-9_-]"), "-");
    // Éviter les doublons de séparateurs
    result.replace(QRegExp("-+"), "-");
    // Supprimer les tirets au début et à la fin
    result = result.trimmed();
    while (result.startsWith("-")) {
        result = result.mid(1);
    }
    while (result.endsWith("-")) {
        result = result.left(result.length() - 1);
    }
    return result;
}

QString getCommandNameMappingFilePath() {
    QDir configDir = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    configDir.mkpath("appimagelauncher");
    return configDir.absoluteFilePath("appimagelauncher/command_mappings.conf");
}

void registerCommandNameMapping(const QString& pathToAppImage, const QString& commandName) {
    QSettings mappings(getCommandNameMappingFilePath(), QSettings::IniFormat);
    mappings.setValue(pathToAppImage, commandName);
}

QString getCommandNameMapping(const QString& pathToAppImage) {
    QSettings mappings(getCommandNameMappingFilePath(), QSettings::IniFormat);
    return mappings.value(pathToAppImage).toString();
}

void removeCommandNameMapping(const QString& pathToAppImage) {
    QSettings mappings(getCommandNameMappingFilePath(), QSettings::IniFormat);
    mappings.remove(pathToAppImage);
}

QString extractCommandName(const QString& pathToAppImage) {
    // 1. Extraire le nom du fichier .desktop à partir du chemin de l'AppImage
    char* desktopFilePath = nullptr;
    
    // On génère le hash MD5 comme AppImageLauncher le fait
    std::string pathStr = pathToAppImage.toStdString();
    const char* path = pathStr.c_str();
    unsigned char* digest = (unsigned char*)appimage_get_md5(path);
    
    if (digest != nullptr) {
        char* id = appimage_hexlify((const char*)digest, 16);
        desktopFilePath = appimage_registered_desktop_file_path(path, id, false);
        free(id);
        free(digest);
    }
    
    QString commandName;
    
    // 2. Si le fichier .desktop existe, on l'analyse
    if (desktopFilePath != nullptr && QFile::exists(desktopFilePath)) {
        QSettings desktopFile(QString(desktopFilePath), QSettings::IniFormat);
        desktopFile.beginGroup("Desktop Entry");
        
        // 3. Essayer différents champs dans l'ordre de priorité
        QString execField = desktopFile.value("Exec").toString();
        QString nameField = desktopFile.value("Name").toString();
        
        // 4. Nettoyer le champ Exec pour obtenir juste le nom de la commande
        if (!execField.isEmpty()) {
            // Extraire le premier mot avant les arguments
            QString command = execField.split(" ").first();
            // Supprimer chemins et extensions
            command = QFileInfo(command).baseName();
            if (!command.isEmpty()) {
                commandName = sanitizeCommandName(command);
                free(desktopFilePath);
                return commandName;
            }
        }
        
        // 5. Utiliser le champ Name comme fallback
        if (!nameField.isEmpty()) {
            commandName = sanitizeCommandName(nameField);
            free(desktopFilePath);
            return commandName;
        }
        
        free(desktopFilePath);
    }
    
    // 6. Fallback sur le nom du fichier AppImage
    commandName = sanitizeCommandName(QFileInfo(pathToAppImage).baseName());
    
    return commandName;
}
