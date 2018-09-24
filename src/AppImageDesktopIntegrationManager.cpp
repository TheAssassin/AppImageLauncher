
#include <iostream>
#include <sstream>
#include <QDebug>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QRegularExpression>
#include <QDirIterator>
#include <QJsonParseError>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QDataStream>

#include <appimage/appimage.h>

#include <translationmanager.h>
#include "AppImageDesktopIntegrationManager.h"
#include "AppImageLauncherConfig.h"

QDir AppImageDesktopIntegrationManager::integratedAppImagesDir;

void AppImageDesktopIntegrationManager::integrateAppImage(const QString &pathToAppImage) {
    auto pathToIntegratedAppImage = buildDeploymentPath(pathToAppImage);
    tryMoveAppImage(pathToAppImage, pathToIntegratedAppImage);

    if (!installDesktopFile(pathToIntegratedAppImage, true))
        throw IntegrationFailed(QObject::tr("Unable to install the AppImage Desktop File.").toStdString());

    // make sure the icons in the launcher are refreshed
    if (!AppImageDesktopIntegrationManager::updateDesktopDatabaseAndIconCaches())
        throw IntegrationFailed(QObject::tr("Unable to update Desktop Database and/or Icons").toStdString());
}

void AppImageDesktopIntegrationManager::tryMoveAppImage(const QString &pathToAppImage,
                                                        const QString &pathToIntegratedAppImage) const {
    // check whether integration was successful
    // need std::strings to get working pointers with .c_str()
    const auto oldPath = pathToAppImage.toStdString();
    const auto newPath = pathToIntegratedAppImage.toStdString();

    // create target directory
    QDir().mkdir(QFileInfo(QFile(pathToIntegratedAppImage)).dir().absolutePath());

    // check whether AppImage is in integration directory already
    if (QFileInfo(pathToAppImage).absoluteFilePath() != QFileInfo(pathToIntegratedAppImage).absoluteFilePath()) {
        // need to check whether file exists
        // if it does, the existing AppImage needs to be removed before rename can be called
        if (QFile(pathToIntegratedAppImage).exists())
            throw OverridingExistingAppImageFile("");

        bool succeed = QFile::rename(pathToAppImage, pathToIntegratedAppImage);
        qWarning() << QObject::tr("Unable to move %1 to %2, trying coping it instead.").arg(pathToAppImage,
                                                                                            pathToIntegratedAppImage);
        if (!succeed)
            succeed = QFile::copy(pathToAppImage, pathToIntegratedAppImage);

        if (!succeed)
            throw IntegrationFailed(QObject::tr("Unable to move or copy AppImage to %1.")
                                            .arg(integratedAppImagesDir.path()).toStdString());
    }
}

QString AppImageDesktopIntegrationManager::buildDeploymentPath(const QString &pathToAppImage) {
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

    return AppImageLauncherConfig::getIntegratedAppImagesDir() + "/" + fileName;
}

bool AppImageDesktopIntegrationManager::hasAlreadyBeenIntegrated(const QString &pathToAppImage) {
    return appimage_is_registered_in_system(pathToAppImage.toStdString().c_str());
}

bool AppImageDesktopIntegrationManager::installDesktopFile(const QString &pathToAppImage, bool resolveCollisions) {
    if (appimage_register_in_system(pathToAppImage.toStdString().c_str(), false) != 0)
        throw IntegrationFailed(QObject::tr("Unable to create desktop file.").toStdString());

    const auto *desktopFilePath = appimage_registered_desktop_file_path(pathToAppImage.toStdString().c_str(),
                                                                        nullptr, false);

    // sanity check -- if the file doesn't exist, the function returns NULL
    if (desktopFilePath == nullptr)
        throw IntegrationFailed(QObject::tr("Desktop file was not created.").toStdString());

    // check that file exists
    if (!QFile(desktopFilePath).exists())
        throw IntegrationFailed(QObject::tr("Desktop file was not created.").toStdString());

    /* write AppImageLauncher specific entries to desktop file
     *
     * unfortunately, QSettings doesn't work as a desktop file reader/writer, and libqtxdg isn't really meant to be
     * used by projects via add_subdirectory/ExternalProject
     * a system dependency is not an option for this project, and we link to glib already anyway, so let's just use
     * glib, which is known to work
     */

    std::shared_ptr<GKeyFile> desktopFile(g_key_file_new(), g_key_file_free);

    std::shared_ptr<GError *> error(nullptr, g_error_free);

    const auto flags = GKeyFileFlags(G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS);

    if (!g_key_file_load_from_file(desktopFile.get(), desktopFilePath, flags, error.get()))
        throw IntegrationFailed(QObject::tr("Failed to load desktop file: %1")
                                        .arg(QString::fromLocal8Bit((*error)->message)).toStdString());


    const auto *nameEntry = g_key_file_get_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP,
                                                  G_KEY_FILE_DESKTOP_KEY_NAME, error.get());

    if (nameEntry == nullptr)
        throw IntegrationFailed(QObject::tr("AppImage has invalid desktop file.").toStdString());

    if (resolveCollisions)
        resolveDesktopFileCollisions(desktopFilePath, desktopFile, nameEntry);

    char const *desktopActions[] = {"Remove", "Update"};

    g_key_file_set_string_list(
            desktopFile.get(),
            G_KEY_FILE_DESKTOP_GROUP,
            G_KEY_FILE_DESKTOP_KEY_ACTIONS,
            desktopActions, 2
    );

    // load translations from JSON file(s)
    QMap<QString, QString> removeActionNameTranslations;
    QMap<QString, QString> updateActionNameTranslations;

    {
        QDirIterator i18nDirIterator(TranslationManager::getTranslationDir());

        while (i18nDirIterator.hasNext()) {
            const auto &filePath = i18nDirIterator.next();
            const auto &fileName = QFileInfo(filePath).fileName();
            QJsonObject jsonObj = readTranslationsFile(filePath, fileName);

            // parse locale from filename
            auto locale = filePath.section('.', 1);

            for (const auto &key : jsonObj.keys()) {
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

        g_key_file_set_string(desktopFile.get(), removeSectionName, "Name", "Remove AppImage from system");

        std::ostringstream removeExecPath;
        removeExecPath << CMAKE_INSTALL_PREFIX << "/lib/appimagelauncher/remove " << pathToAppImage.toStdString();
        g_key_file_set_string(desktopFile.get(), removeSectionName, "Exec", removeExecPath.str().c_str());

        // install translations
        auto it = QMapIterator<QString, QString>(removeActionNameTranslations);
        while (it.hasNext()) {
            auto entry = it.next();
            g_key_file_set_locale_string(desktopFile.get(), removeSectionName, "Name",
                                         entry.key().toStdString().c_str(), entry.value().toStdString().c_str());
        }
    }

    // add Update action
    {
        const auto updateSectionName = "Desktop Action Update";

        g_key_file_set_string(desktopFile.get(), updateSectionName, "Name", "Update AppImage");

        std::ostringstream updateExecPath;
        updateExecPath << CMAKE_INSTALL_PREFIX << "/lib/appimagelauncher/update " << pathToAppImage.toStdString();
        g_key_file_set_string(desktopFile.get(), updateSectionName, "Exec", updateExecPath.str().c_str());

        // install translations
        auto it = QMapIterator<QString, QString>(updateActionNameTranslations);
        while (it.hasNext()) {
            auto entry = it.next();
            g_key_file_set_locale_string(desktopFile.get(), updateSectionName, "Name",
                                         entry.key().toStdString().c_str(), entry.value().toStdString().c_str());
        }
    }

    // add version key
    const auto version = QApplication::applicationVersion().replace("version ", "").toStdString();
    g_key_file_set_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, "X-AppImageLauncher-Version", version.c_str());

    if (!g_key_file_save_to_file(desktopFile.get(), desktopFilePath, error.get()))
        throw IntegrationFailed(QObject::tr("Failed to save desktop file: %1")
                                        .arg(QString::fromLocal8Bit((*error)->message)).toStdString());

    // notify KDE/Plasma about icon change
    {
        auto message = QDBusMessage::createSignal(QStringLiteral("/KIconLoader"), QStringLiteral("org.kde.KIconLoader"),
                                                  QStringLiteral("iconChanged"));
        message.setArguments({0});
        QDBusConnection::sessionBus().send(message);
    }

    return true;
}

QJsonObject
AppImageDesktopIntegrationManager::readTranslationsFile(const QString &filePath, const QString &fileName) const {
    if (!QFileInfo(filePath).isFile() || !(fileName.startsWith("desktopfiles.") && fileName.endsWith(".json")))
        return QJsonObject();

    // check whether filename's format is alright, otherwise parsing the locale might try to access a
    // non-existing (or the wrong) member
    auto splitFilename = fileName.split(".");

    if (splitFilename.size() != 3)
        return QJsonObject();

    QFile jsonFile(filePath);

    if (!jsonFile.open(QIODevice::ReadOnly))
        qWarning() << QObject::tr("Could not parse desktop file translations: Could not open file for reading %1")
                .arg(fileName);

    // TODO: need to make sure that this doesn't try to read huge files at once
    auto data = jsonFile.readAll();

    QJsonParseError parseError{};
    auto jsonDoc = QJsonDocument::fromJson(data, &parseError);

    // show warning on syntax errors and continue
    if (parseError.error != QJsonParseError::NoError || jsonDoc.isNull() || !jsonDoc.isObject())
        qWarning() << QObject::tr("Could not parse desktop file translations: Invalid syntax %1")
                .arg(parseError.errorString());

    return jsonDoc.object();
}

void AppImageDesktopIntegrationManager::resolveDesktopFileCollisions(const char *desktopFilePath,
                                                                     const std::shared_ptr<GKeyFile> &desktopFile,
                                                                     const gchar *nameEntry) {// TODO: support multilingual collisions
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

        for (const auto &fileName : collisions) {
            const auto &currentNameEntry = collisions[fileName];

            auto match = regex.match(currentNameEntry);

            if (match.hasMatch()) {
                const unsigned int num = match.captured(0).toUInt();
                if (num >= currentNumber)
                    currentNumber = num + 1;
            }
        }

        auto newName = QString(nameEntry) + " (" + QString::number(currentNumber) + ")";
        g_key_file_set_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME,
                              newName.toStdString().c_str());
    }
}

void AppImageDesktopIntegrationManager::updateAppImage(const QString &pathToAppImage) {
    bool result = installDesktopFile(pathToAppImage, true);
    if (!result)
        throw IntegrationFailed(QObject::tr("Unable to update AppImage Desktop file.").toStdString());
}

void AppImageDesktopIntegrationManager::removeAppImageIntegration(const QString &appImagePath) {
    if (appimage_unregister_in_system(appImagePath.toStdString().c_str(), false) != 0) {
        throw AppImageIntegrationRemovalFailed(
                QObject::tr("Unable to remove AppImage Desktop integration files.").toStdString());
    }
}

bool AppImageDesktopIntegrationManager::isPlacedInTheDefaultAppsDir(const QString &pathToAppImage) {
    return integratedAppImagesDir == QFileInfo(pathToAppImage).absoluteDir();
}

AppImageDesktopIntegrationManager::AppImageDesktopIntegrationManager() {
    integratedAppImagesDir = AppImageLauncherConfig::getIntegratedAppImagesDir();
}

const QDir &AppImageDesktopIntegrationManager::getIntegratedAppImagesDir() const {
    return integratedAppImagesDir;
}

const QString AppImageDesktopIntegrationManager::getIntegratedAppImagesDirPath() const {
    return integratedAppImagesDir.path();
}

bool AppImageDesktopIntegrationManager::updateDesktopDatabaseAndIconCaches() {
    auto commands = {
            "update-desktop-database ~/.local/share/applications",
            "gtk-update-icon-cache-3.0 ~/.local/share/icons/hicolor/ -t",
            "gtk-update-icon-cache ~/.local/share/icons/hicolor/ -t",
            "xdg-desktop-menu forceupdate",
    };

    for (const auto &command : commands) {
        // exit codes are not evaluated intentionally
        system(command);
    }

    return true;
}

QMap<QString, QString> AppImageDesktopIntegrationManager::findCollisions(const QString &currentNameEntry) {
    QMap<QString, QString> collisions;

    // default locations of desktop files on systems
    const auto directories = {QString("/usr/share/applications/"), QString(xdg_data_home()) + "/applications/"};

    for (const auto &directory : directories) {
        QDirIterator iterator(directory, QDirIterator::FollowSymlinks);

        while (iterator.hasNext()) {
            const auto &filename = iterator.next();

            if (!QFileInfo(filename).isFile() || !filename.endsWith(".desktop"))
                continue;

            std::shared_ptr<GKeyFile> desktopFile(g_key_file_new(), g_key_file_free);
            std::shared_ptr<GError *> error(nullptr, g_error_free);

            // if the key file parser can't load the file, it's most likely not a valid desktop file, so we just skip this file
            if (!g_key_file_load_from_file(desktopFile.get(), filename.toStdString().c_str(),
                                           G_KEY_FILE_KEEP_TRANSLATIONS, error.get()))
                continue;

            auto *nameEntry = g_key_file_get_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP,
                                                    G_KEY_FILE_DESKTOP_KEY_NAME, error.get());

            // invalid desktop file, needs to be skipped
            if (nameEntry == nullptr)
                continue;

            if (QString(nameEntry).trimmed().startsWith(currentNameEntry.trimmed())) {
                collisions[filename] = QString(nameEntry);
            }
        }
    }

    return collisions;
}

QString AppImageDesktopIntegrationManager::getAppImageDigestMd5(const QString &pathToAppImage) {
    QCryptographicHash hash(QCryptographicHash::Md5);
    QFile f(pathToAppImage);
    if (f.open(QIODevice::ReadOnly)) {
        QDataStream in(&f);
        char buff[1024];
        while (!in.atEnd()) {
            int bytesRead = in.readRawData(buff, 1024);
            hash.addData(buff, bytesRead);
        }

        return hash.result().toHex().toUpper();
    }
    return QString();
}

bool AppImageDesktopIntegrationManager::cleanUpOldDesktopIntegrationResources(bool verbose) {
    auto dirPath = QString(xdg_data_home()) + "/applications";

    auto directory = QDir(dirPath);

    QStringList filters;
    filters << "appimagekit_*.desktop";

    directory.setNameFilters(filters);

    for (auto desktopFilePath : directory.entryList()) {
        desktopFilePath = dirPath + "/" + desktopFilePath;

        std::shared_ptr<GKeyFile> desktopFile(g_key_file_new(), [](GKeyFile *p) {
            g_key_file_free(p);
        });

        if (!g_key_file_load_from_file(desktopFile.get(), desktopFilePath.toStdString().c_str(), G_KEY_FILE_NONE,
                                       nullptr)) {
            continue;
        }

        std::shared_ptr<char> execValue(
                g_key_file_get_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_EXEC,
                                      nullptr), [](char *p) {
                    free(p);
                });

        // if there is no Exec value in the file, the desktop file is apparently broken, therefore we skip the file
        if (execValue == nullptr) {
            continue;
        }

        std::shared_ptr<char> tryExecValue(
                g_key_file_get_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_TRY_EXEC,
                                      nullptr), [](char *p) {
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
                std::cout << "AppImage no longer exists, cleaning up resources: " << appImagePath.toStdString()
                          << std::endl;

            if (verbose)
                std::cout << "Removing desktop file: " << desktopFilePath.toStdString() << std::endl;

            QFile(desktopFilePath).remove();

            // TODO: clean up related resources such as icons or MIME definitions

            auto *iconValue = g_key_file_get_string(desktopFile.get(), G_KEY_FILE_DESKTOP_GROUP,
                                                    G_KEY_FILE_DESKTOP_KEY_ICON, nullptr);

            if (iconValue != nullptr) {
                for (QDirIterator it("~/.local/share/icons/", QDirIterator::Subdirectories); it.hasNext();) {
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
