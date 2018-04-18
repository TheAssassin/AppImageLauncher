// own header
#include "shared.h"

// system includes
#include <iostream>
#include <sstream>
extern "C" {
    #include <appimage/appimage.h>
    #include <glib.h>
    // #include <libgen.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <xdg-basedir.h>
}

// library includes
#include <QDirIterator>
#include <QMap>
#include <QMessageBox>
#include <QRegularExpression>


bool makeExecutable(const std::string& path) {
    struct stat fileStat{};

    if (stat(path.c_str(), &fileStat) != 0) {
        std::cerr << "Failed to call stat() on " << path << std::endl;
        return false;
    }

    // no action required when file is executable already
    // this could happen in scenarios when an AppImage is in a read-only location
    if ((fileStat.st_uid == getuid() && fileStat.st_mode & 0100) ||
        (fileStat.st_gid == getgid() && fileStat.st_mode & 0010) ||
        (fileStat.st_mode & 0001)) {
        return true;
    }

    return chmod(path.c_str(), fileStat.st_mode | 0111) == 0;
}


QString buildPathToIntegratedAppImage(const QString& pathToAppImage) {
    return integratedAppImagesDestination + basename(const_cast<char*>(pathToAppImage.toStdString().c_str()));
}


QMap<QString, QString> findCollisions(const QString& currentNameEntry) {
    QMap<QString, QString> collisions;

    // default locations of desktop files on systems
    const auto directories = {QString("/usr/share/applications/"), QString(xdg_data_home()) + "/applications/"};

    for (const auto& directory : directories) {
        QDirIterator iterator(directory, QDirIterator::FollowSymlinks);

        while (iterator.hasNext()) {
            const auto& filename = iterator.next();

            if (!QFileInfo(filename).isFile() || !filename.endsWith(".desktop"))
                continue;

            GKeyFile* desktopFile = g_key_file_new();
            GError* error = nullptr;

            auto cleanup = [&desktopFile, &error]() {
                if (desktopFile != nullptr) {
                    g_key_file_free(desktopFile);
                    desktopFile = nullptr;
                }

                if (error != nullptr) {
                    g_error_free(error);
                    error = nullptr;
                }
            };

            // if the key file parser can't load the file, it's most likely not a valid desktop file, so we just skip this file
            if (!g_key_file_load_from_file(desktopFile, filename.toStdString().c_str(), G_KEY_FILE_KEEP_TRANSLATIONS, &error)) {
                cleanup();
                continue;
            }

            auto* nameEntry = g_key_file_get_string(desktopFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, &error);

            // invalid desktop file, needs to be skipped
            if (nameEntry == nullptr) {
                cleanup();
                continue;
            }

            if (QString(nameEntry).trimmed().startsWith(currentNameEntry.trimmed())) {
                collisions[filename] = QString(nameEntry);
            }

            cleanup();
        }
    }

    return collisions;
}


bool integrateAppImage(const QString& pathToAppImage, const QString& pathToIntegratedAppImage) {
    // need std::strings to get working pointers with .c_str()
    const auto oldPath = pathToAppImage.toStdString();
    const auto newPath = pathToIntegratedAppImage.toStdString();

    // create target directory
    QDir().mkdir(QFileInfo(QFile(pathToIntegratedAppImage)).dir().absolutePath());

    // need to check whether file exists
    // if it does, the existing AppImage needs to be removed before rename can be called
    if (QFile(pathToIntegratedAppImage).exists()) {
        QFile(pathToIntegratedAppImage).remove();
    }
    if (!QFile(pathToAppImage).rename(pathToIntegratedAppImage)) {
        QMessageBox::critical(nullptr, "Error", "Failed to move AppImage to target location");
        return false;
    }

    if (appimage_register_in_system(newPath.c_str(), false) != 0)
        return false;

    const auto* desktopFilePath = appimage_registered_desktop_file_path(newPath.c_str(), nullptr, false);

    // sanity check -- if the file doesn't exist, the function returns NULL
    if (desktopFilePath == nullptr) {
        QMessageBox::critical(nullptr, "Error", "Failed to find integrated desktop file");
        return false;
    }

    // check that file exists
    if (!QFile(desktopFilePath).exists())
        return false;

    /* write AppImageLauncher specific entries to desktop file
     *
     * unfortunately, QSettings doesn't work as a desktop file reader/writer, and libqtxdg isn't really meant to be
     * used by projects via add_subdirectory/ExternalProject
     * a system dependency is not an option for this project, and we link to glib already anyway, so let's just use
     * glib, which is known to work
     */
    auto desktopFile = g_key_file_new();

    GError *error = nullptr;
    const auto flags = GKeyFileFlags(G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS);

    // for people who don't want to create memory leaks when using C APIs, C++11's lambdas provide a great option to
    // clean up those data
    auto cleanup = [&error, &desktopFile]() {
        if (desktopFile != nullptr)
            g_key_file_free(desktopFile);

        if (error != nullptr)
            g_error_free(error);
    };

    auto handleError = [&error, &desktopFile, &cleanup]() {
        std::ostringstream ss;
        ss << "Failed to load desktop file: " << std::endl << error->message;
        QMessageBox::critical(nullptr, "Error", QString::fromStdString(ss.str()));

        cleanup();
    };

    if (!g_key_file_load_from_file(desktopFile, desktopFilePath, flags, &error)) {
        handleError();
        return false;
    }

    const auto* nameEntry = g_key_file_get_string(desktopFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, &error);

    if (nameEntry == nullptr) {
        QMessageBox::warning(nullptr, "Warning", "AppImage has invalid desktop file");
    }

    // TODO: support multilingual collisions
    auto collisions = findCollisions(nameEntry);

    // make sure to remove own entry
    collisions.remove(QString(desktopFilePath));

    if (!collisions.empty()) {
        // collisions are resolved like in the filesystem: a monotonically increasing number in brackets is appended to
        // the Name
        // in order to keep the number monotonically increasing, we look for the highest number in brackets in the
        // existing entries, add 1 to it, and append it in brackets to the current desktop file's Name entry

        unsigned int currentNumber = 1;

        QRegularExpression regex("^.*([0-9]+)$");

        for (const auto& fileName : collisions) {
            const auto& currentNameEntry = collisions[fileName];

            auto match = regex.match(currentNameEntry);

            if (match.hasMatch()) {
                const unsigned int num = match.captured(0).toUInt();
                if (num >= currentNumber)
                    currentNumber = num + 1;
            }
        }

        auto newName = QString(nameEntry) + " (" + QString::number(currentNumber) + ")";
        g_key_file_set_string(desktopFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, newName.toStdString().c_str());
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

    std::vector<std::string> desktopActions = {"Remove", "Update"};

    g_key_file_set_string_list(
        desktopFile,
        G_KEY_FILE_DESKTOP_GROUP,
        G_KEY_FILE_DESKTOP_KEY_ACTIONS,
        convertToCharPointerList(desktopActions).data(),
        desktopActions.size()
    );

    // add Remove action
    {
        const auto removeSectionName = "Desktop Action Remove";

        g_key_file_set_string(desktopFile, removeSectionName, "Name", "Remove AppImage from system");

        std::ostringstream removeExecPath;
        removeExecPath << CMAKE_INSTALL_PREFIX << "/lib/appimagelauncher/remove " << newPath;
        g_key_file_set_string(desktopFile, removeSectionName, "Exec", removeExecPath.str().c_str());
    }

    // add Update action
    {
        const auto updateSectionName = "Desktop Action Update";

        g_key_file_set_string(desktopFile, updateSectionName, "Name", "Update AppImage");

        std::ostringstream updateExecPath;
        updateExecPath << CMAKE_INSTALL_PREFIX << "/lib/appimagelauncher/update " << newPath;
        g_key_file_set_string(desktopFile, updateSectionName, "Exec", updateExecPath.str().c_str());
    }

    if (!g_key_file_save_to_file(desktopFile, desktopFilePath, &error)) {
        handleError();
        return false;
    }

    cleanup();
    return true;
}
