// system includes
#include <iostream>
#include <memory>
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
#include <QSettings>
#include <QStandardPaths>

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


std::shared_ptr<QSettings> getConfig() {
    // calculate path to config file
    const auto configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    const auto configFilePath = configPath + "/appimagelauncher.cfg";

    // if the file does not exist, we'll just use the standard location
    // while in theory it would have been possible to just write the default location to the file, if we'd ever change
    // it again, we'd leave a lot of systems in the old state, and would have to write some complex code to resolve
    // the situation
    // therefore, the file is simply created, but left empty intentionally
    if (!QFileInfo::exists(configFilePath)) {
        QFile file(configFilePath);
        file.open(QIODevice::WriteOnly);
        file.write("[AppImageLauncher]\n"
                   "# destination = ~/Applications\n"
                   "# enable_daemon = true\n");
    }

    return std::make_shared<QSettings>(configFilePath, QSettings::IniFormat);
}


QDir integratedAppImagesDestination() {
    auto config = getConfig();

    static const QString keyName("AppImageLauncher/destination");
    if (config->contains(keyName))
        return config->value(keyName).toString();

    return DEFAULT_INTEGRATION_DESTINATION;
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
                collisions[filename] = QString(nameEntry);
            }
        }
    }

    return collisions;
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

bool hasAlreadyBeenIntegrated(const QString& pathToAppImage) {
    return appimage_is_registered_in_system(pathToAppImage.toStdString().c_str());
}

bool cleanUpOldDesktopIntegrationResources(bool verbose) {
    auto dirPath = QString(xdg_data_home()) + "/applications";

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
