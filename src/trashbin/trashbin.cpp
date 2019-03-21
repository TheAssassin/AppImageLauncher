// system includes
#include <iostream>
#include <sys/stat.h>

// library includes
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <appimage/appimage.h>

// local includes
#include "trashbin.h"
#include "shared.h"

class TrashBin::PrivateData {
    public:
        const QDir dir;

    public:
        PrivateData() : dir(integratedAppImagesDestination().path() + "/.trash") {
            // make sure trash directory exists
            QDir(integratedAppImagesDestination().path()).mkdir(".trash");
        }

        bool canBeCleanedUp(const QString& path) {
            return true;
        }
};

TrashBin::TrashBin() {
    d = new PrivateData();
}

QString TrashBin::path() {
    return d->dir.path();
}

bool TrashBin::disposeAppImage(const QString& pathToAppImage) {
    if (!QFile(pathToAppImage).exists()) {
        std::cerr << "No such file or directory: " << pathToAppImage.toStdString() << std::endl;
        return false;
    }

    // moving AppImages into the trash bin might fail if there's a file with the same filename
    // removing multiple files with the same filenames is a valid use case, though
    // therefore, a timestamp shall be prepended to the filename
    // it is very unlike that some user will remove more than a AppImage per second, but if that should be the case,
    // we could eventually increase the precision of the timestamp
    // for now, this is not necessary
    auto timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    auto newPath = d->dir.path() + QString("/") + timestamp + "_" + QFileInfo(pathToAppImage).fileName();

    if (!QFile(pathToAppImage).rename(newPath))
        return false;

    if (!makeNonExecutable(newPath))
        return false;

    return true;
}

bool TrashBin::cleanUp() {
    for (QDirIterator iterator(d->dir, QDirIterator::FollowSymlinks); iterator.hasNext();) {
        auto currentPath = iterator.next();

        if (!QFileInfo(currentPath).isFile())
            continue;

        if (appimage_get_type(currentPath.toStdString().c_str(), false) <= 0)
            continue;

        if (!d->canBeCleanedUp(currentPath)) {
            std::cerr << "Cannot clean up AppImage yet: " << currentPath.toStdString() << std::endl;
            continue;
        }

        std::cerr << "Removing AppImage: " << currentPath.toStdString() << std::endl;

        // silently ignore if files can not be removed
        // they shall be removed on subsequent runs
        // if this won't happen and the trash directory will only get bigger at some point, we might need to
        // reconsider this decision
        if (!QFile(currentPath).remove())
            continue;
    }

    return true;
}
