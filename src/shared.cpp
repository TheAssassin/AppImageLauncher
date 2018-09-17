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
