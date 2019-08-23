// system includes
#include <deque>
#include <set>
#include <iostream>
#include <sys/stat.h>

// library includes
#include <QCoreApplication>
#include <QDirIterator>
#include <QMutex>
#include <QTimer>
#include <appimage/appimage.h>

// local includes
#include "shared.h"
#include "filesystemwatcher.h"
#include "worker.h"

/**
 * Read the modification time of the file pointed by <filePath>
 * @param filePath
 * @return file modification time
 */
long readFileModificationTime(char* filePath) {
    struct stat attrib = {0x0};
    stat(filePath, &attrib);
    return attrib.st_ctime;
}

/**
 * Monitors whether the application binary has changed since the process was started. In such case the application
 * is restarted.
 *
 * @param argv
 */
QTimer* setupBinaryUpdatesMonitor(char* const* argv) {
    auto* timer = new QTimer();
    // It's used to restart the daemon if the binary changes
    static const long binaryModificationTime = readFileModificationTime(argv[0]);

    // callback to compare and restart the app if the binary changed since it was started
    QObject::connect(timer, &QTimer::timeout, [argv]() {
        long newBinaryModificationTime = readFileModificationTime(argv[0]);
        if (newBinaryModificationTime != binaryModificationTime) {
            std::cerr << "Binary file changed since the applications was started, proceeding to restart it."
                      << std::endl;
            execv(argv[0], argv);
        }
    });

    // check every 5 min
    timer->setInterval(5 * 60 * 1000);
    return timer;

}

int main(int argc, char* argv[]) {
    // make sure shared won't try to use the UI
    setenv("_FORCE_HEADLESS", "1", 1);

    QCoreApplication app(argc, argv);

    // watchers are kind of value objects, the watched directories may not change over the lifetime of the object
    // therefore we need to create a set beforehand, containing all directories we want to have watched
    QSet<QString> watchedDirectories;

    // of course we need to watch the main integration directory
    const auto defaultDestination = integratedAppImagesDestination();
    watchedDirectories.insert(defaultDestination.absolutePath());

    // however, there's likely additional ones to watch
    const auto additionalDirs = additionalAppImagesLocations();
    for (const auto& d : additionalDirs) {
        watchedDirectories.insert(QDir(d).absolutePath());
    }

    // time to create the watcher object
    FileSystemWatcher watcher(watchedDirectories);

    // create a daemon worker instance
    // it is used to integrate all AppImages initially, and to integrate files found via inotify
    Worker worker;

    // initial search for AppImages; if AppImages are found, they will be integrated, unless they already are
    std::cout << "Searching for existing AppImages" << std::endl;
    for (const auto& dir : watcher.directories()) {
        std::cout << "Searching directory: " << dir.toStdString() << std::endl;
        for (QDirIterator it(dir); it.hasNext();) {
            const auto& path = it.next();
            if (QFileInfo(path).isFile()) {
                const auto appImageType = appimage_get_type(path.toStdString().c_str(), false);
                const auto isAppImage = 0 < appImageType && appImageType <= 2;
                if (isAppImage) {
                    // at application startup, we don't want to integrate AppImages that have been integrated already,
                    // as that it slows down very much
                    // the integration will be updated as soon as any of these AppImages is run with AppImageLauncher
                    std::cout << "Found AppImage: " << path.toStdString() << std::endl;
                    if (!appimage_is_registered_in_system(path.toStdString().c_str())) {
                        std::cout << "AppImage is not integrated yet, integrating" << std::endl;
                        worker.scheduleForIntegration(path);
                    } else if (!desktopFileHasBeenUpdatedSinceLastUpdate(path)) {
                        std::cout << "AppImage has been integrated already but needs to be reintegrated" << std::endl;
                        worker.scheduleForIntegration(path);
                    } else {
                        std::cout << "AppImage integrated already, skipping" << std::endl;
                    }
                }
            }
        }
    }
    // (re-)integrate all AppImages at once
    worker.executeDeferredOperations();

    // after (re-)integrating all AppImages, clean up old desktop integration resources before start
    if (!cleanUpOldDesktopIntegrationResources()) {
        std::cout << "Failed to clean up old desktop integration resources" << std::endl;
    }

    std::cout << "Watching directories: ";
    for (const auto& dir : watcher.directories()) {
        std::cout << dir.toStdString().c_str() << " ";
    }
    std::cout << std::endl;

    FileSystemWatcher::connect(&watcher, &FileSystemWatcher::fileChanged, &worker, &Worker::scheduleForIntegration,
                               Qt::QueuedConnection);
    FileSystemWatcher::connect(&watcher, &FileSystemWatcher::fileRemoved, &worker, &Worker::scheduleForUnintegration,
                               Qt::QueuedConnection);

    if (!watcher.startWatching()) {
        std::cerr << "Could not start watching directories" << std::endl;
        return 1;
    }

    watcher.startWatching();
    QObject::connect(&app, &QCoreApplication::aboutToQuit, &watcher, &FileSystemWatcher::stopWatching);

    QTimer* binaryUpdatesMonitor = setupBinaryUpdatesMonitor(argv);
    binaryUpdatesMonitor->start();

    return QCoreApplication::exec();
}
