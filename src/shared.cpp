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
#include <QDebug>
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

// local headers
#include "shared.h"
#include "translationmanager.h"

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
    // if type 2 AppImage, we can build a "content-aware" filename
    // see #7 for details
    auto digest = getAppImageDigestMd5(pathToAppImage);

    const QFileInfo appImageInfo(pathToAppImage);

    QString baseName = appImageInfo.completeBaseName();

    // if digest is available, append a separator
    if (!digest.isEmpty())
        baseName += "_" + digest;

    auto fileName = baseName;

    if (!appImageInfo.completeSuffix().isEmpty())
        fileName += "." + appImageInfo.completeSuffix();

    return integratedAppImagesDestination + "/" + fileName;
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

bool installDesktopFile(const QString& pathToAppImage, bool resolveCollisions) {
    if (appimage_register_in_system(pathToAppImage.toStdString().c_str(), false) != 0) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Error"),
            QObject::tr("Failed to register AppImage in system via libappimage")
        );
        return INTEGRATION_FAILED;
    }

    const auto* desktopFilePath = appimage_registered_desktop_file_path(pathToAppImage.toStdString().c_str(), nullptr, false);

    // sanity check -- if the file doesn't exist, the function returns NULL
    if (desktopFilePath == nullptr) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Error"),
            QObject::tr("Failed to find integrated desktop file")
        );
        return INTEGRATION_FAILED;
    }

    // check that file exists
    if (!QFile(desktopFilePath).exists()) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Error"),
            QObject::tr("Couldn't find integrated AppImage's desktop file")
        );
        return INTEGRATION_FAILED;
    }

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
        ss << QObject::tr("Failed to load desktop file:").toStdString() << std::endl << error->message;
        QMessageBox::critical(
            nullptr,
            QObject::tr("Error"),
            QString::fromStdString(ss.str())
        );

        cleanup();
    };

    if (!g_key_file_load_from_file(desktopFile, desktopFilePath, flags, &error)) {
        handleError();
        return INTEGRATION_FAILED;
    }

    const auto* nameEntry = g_key_file_get_string(desktopFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, &error);

    if (nameEntry == nullptr) {
        QMessageBox::warning(
            nullptr,
            QObject::tr("Warning"),
            QObject::tr("AppImage has invalid desktop file")
        );
    }

    if (resolveCollisions) {
        // TODO: support multilingual collisions
        auto collisions = findCollisions(nameEntry);

        // make sure to remove own entry
        collisions.remove(QString(desktopFilePath));

        if (!collisions.empty()) {
            // collisions are resolved like in the filesystem: a monotonically increasing number in brackets is
            // appended to the Name in order to keep the number monotonically increasing, we look for the highest
            // number in brackets in the existing entries, add 1 to it, and append it in brackets to the current
            // desktop file's Name entry

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

    // load translations from JSON file(s)
    QMap<QString, QString> removeActionNameTranslations;
    QMap<QString, QString> updateActionNameTranslations;

    {
        QDirIterator i18nDirIterator(TranslationManager::getTranslationDir());

        while(i18nDirIterator.hasNext()) {
            const auto& filePath = i18nDirIterator.next();
            const auto& fileName = QFileInfo(filePath).fileName();

            auto x = strdup(fileName.toStdString().c_str());

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
                QMessageBox::warning(
                    nullptr,
                    QMessageBox::tr("Warning"),
                    QMessageBox::tr("Could not parse desktop file translations:\nCould not open file for reading:\n\n%1").arg(fileName)
                );
            }

            // TODO: need to make sure that this doesn't try to read huge files at once
            auto data = jsonFile.readAll();

            QJsonParseError parseError{};
            auto jsonDoc = QJsonDocument::fromJson(data, &parseError);

            // show warning on syntax errors and continue
            if (parseError.error != QJsonParseError::NoError || jsonDoc.isNull() || !jsonDoc.isObject()) {
                QMessageBox::warning(
                    nullptr,
                    QMessageBox::tr("Warning"),
                    QMessageBox::tr("Could not parse desktop file translations:\nInvalid syntax:\n\n%1").arg(parseError.errorString())
                );
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

    // add Remove action
    {
        const auto removeSectionName = "Desktop Action Remove";

        g_key_file_set_string(desktopFile, removeSectionName, "Name", "Remove AppImage from system");

        std::ostringstream removeExecPath;
        removeExecPath << CMAKE_INSTALL_PREFIX << "/lib/appimagelauncher/remove " << pathToAppImage.toStdString();
        g_key_file_set_string(desktopFile, removeSectionName, "Exec", removeExecPath.str().c_str());

        // install translations
        auto it = QMapIterator<QString, QString>(removeActionNameTranslations);
        while (it.hasNext()) {
            auto entry = it.next();
            g_key_file_set_locale_string(desktopFile, removeSectionName, "Name", entry.key().toStdString().c_str(), entry.value().toStdString().c_str());
        }
    }

    // add Update action
    {
        const auto updateSectionName = "Desktop Action Update";

        g_key_file_set_string(desktopFile, updateSectionName, "Name", "Update AppImage");

        std::ostringstream updateExecPath;
        updateExecPath << CMAKE_INSTALL_PREFIX << "/lib/appimagelauncher/update " << pathToAppImage.toStdString();
        g_key_file_set_string(desktopFile, updateSectionName, "Exec", updateExecPath.str().c_str());

        // install translations
        auto it = QMapIterator<QString, QString>(updateActionNameTranslations);
        while (it.hasNext()) {
            auto entry = it.next();
            g_key_file_set_locale_string(desktopFile, updateSectionName, "Name", entry.key().toStdString().c_str(), entry.value().toStdString().c_str());
        }
    }

    if (!g_key_file_save_to_file(desktopFile, desktopFilePath, &error)) {
        handleError();
        return false;
    }

    cleanup();

    return true;
}

bool updateDesktopFile(const QString& pathToAppImage) {
    return installDesktopFile(pathToAppImage, true);
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

            auto rv = QMessageBox::warning(
                nullptr,
                QObject::tr("Warning"),
                QString::fromStdString(message.str()),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::Yes
            );

            if (rv == QMessageBox::No) {
                return INTEGRATION_ABORTED;
            }

            QFile(pathToIntegratedAppImage).remove();
        }

        if (!QFile(pathToAppImage).rename(pathToIntegratedAppImage)) {
            auto result = QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("Failed to move AppImage to target location.\n"
                            "Try to copy AppImage instead?"),
                QMessageBox::Ok | QMessageBox::Cancel
            );

            if (result == QMessageBox::Cancel)
                return INTEGRATION_FAILED;

            if (!QFile(pathToAppImage).copy(pathToIntegratedAppImage)) {
                QMessageBox::critical(
                    nullptr,
                    QObject::tr("Error"),
                    QObject::tr("Failed to copy AppImage to target location")
                );
                return INTEGRATION_FAILED;
            }
        }
    }

    if (!installDesktopFile(pathToIntegratedAppImage))
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
    } else {
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
