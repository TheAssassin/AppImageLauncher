// system includes
#include <deque>
#include <set>
#include <iostream>

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

int main(int argc, char* argv[]) {
    // make sure shared won't try to use the UI
    setenv("_FORCE_HEADLESS", "1", 1);

    QCoreApplication app(argc, argv);

    // by default, watch configured/default destination directory only
    const auto defaultDestination = integratedAppImagesDestination();
    FileSystemWatcher watcher(defaultDestination.absolutePath());

    // create a daemon worker instance
    // it is used to integrate all AppImages initially, and to integrate files found via inotify
    Worker worker;

    // initial search for AppImages; if AppImages are found, they will be integrated, unless they already are
    std::cout << "Searching for existing AppImages" << std::endl;
    for (const auto& dir : watcher.directories()) {
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
        std::cout << dir.toStdString().c_str();
    }
    std::cout << std::endl;

    // move worker to thread to allow for integrating files asynchronously
    QThread workerThread;
    worker.moveToThread(&workerThread);
    workerThread.start();

    FileSystemWatcher::connect(&watcher, &FileSystemWatcher::fileChanged, &worker, &Worker::scheduleForIntegration);
    FileSystemWatcher::connect(&watcher, &FileSystemWatcher::fileRemoved, &worker, &Worker::scheduleForUnintegration);

    if (!watcher.startWatching()) {
        std::cerr << "Could not start watching directories" << std::endl;
        return 1;
    }

    // watch directory in a thread
    QThread watcherThread;
    watcher.moveToThread(&watcherThread);
    QThread::connect(&watcherThread, &QThread::started, &watcher, &FileSystemWatcher::readEventsForever);
    QThread::connect(&watcherThread, &QThread::finished, &watcher, &FileSystemWatcher::stopWatching);
    watcherThread.start();

    return app.exec();
}
