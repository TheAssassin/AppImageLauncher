//
// Created by alexis on 9/1/18.
//

// system includes
#include <glib.h>

// library includes
#include <QDebug>
#include <QMessageBox>
#include <appimage/appimage.h>

// local includes
#include "../shared.h"
#include "../translationmanager.h"
#include "AppImageDesktopIntegrationManager.h"

void AppImageDesktopIntegrationManager::integrateAppImage(const QString &pathToAppImage) {
    auto pathToIntegratedAppImage = buildDeploymentPath(pathToAppImage);

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
            throw OverridingExistingFileError("");

        if (!QFile(pathToAppImage).rename(pathToIntegratedAppImage)) {
            auto result = QMessageBox::critical(
                    nullptr,
                    QObject::tr("Error"),
                    QObject::tr("Failed to move AppImage to target location.\n"
                                "Try to copy AppImage instead?"),
                    QMessageBox::Ok | QMessageBox::Cancel
            );

            if (result == QMessageBox::Cancel)
                throw IntegrationFailedError(QObject::tr("Integration aborted by user.").toStdString());

            if (!QFile(pathToAppImage).copy(pathToIntegratedAppImage))
                throw IntegrationFailedError(QObject::tr("Failed to copy AppImage to target location.").toStdString());
        }
    }

    if (!installDesktopFile(pathToIntegratedAppImage, true))
        throw IntegrationFailedError(QObject::tr("Unable to install the AppImage Desktop File.").toStdString());

    // make sure the icons in the launcher are refreshed
    if (!updateDesktopDatabaseAndIconCaches())
        throw IntegrationFailedError(QObject::tr("Unable to update Desktop Database and/or Icons").toStdString());
}

QString AppImageDesktopIntegrationManager::buildDeploymentPath(const QString &pathToAppImage) {
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

bool AppImageDesktopIntegrationManager::hasAlreadyBeenIntegrated(const QString &pathToAppImage) {
    return appimage_is_registered_in_system(pathToAppImage.toStdString().c_str());
}

bool AppImageDesktopIntegrationManager::installDesktopFile(const QString &pathToAppImage, bool resolveCollisions) {
    return ::installDesktopFile(pathToAppImage, resolveCollisions);

}

void AppImageDesktopIntegrationManager::updateAppImage(const QString &pathToAppImage) {
    bool result = ::updateDesktopFile(pathToAppImage);
    if (!result)
        throw IntegrationFailedError(QObject::tr("Unable to update AppImage Desktop file.").toStdString());
}
