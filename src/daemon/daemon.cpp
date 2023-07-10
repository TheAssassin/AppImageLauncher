// STL headers
#include <chrono>

// library headers
#include <QDirIterator>

// local headers
#include "daemon.h"
#include "shared.h"
#include "appimage/appimage.h"

using namespace std::chrono_literals;

#define UPDATE_WATCHED_DIRECTORIES_INTERVAL 30s

namespace {

}

namespace appimagelauncher::daemon {

    Q_LOGGING_CATEGORY(daemonCat, "appimagelauncher.daemon")

    Daemon::Daemon(QObject* parent) : QObject(parent), _settings(getConfig(this)), _worker(new Worker(this)),
                                      _watcher(new FileSystemWatcher(this)), _updateWatchedDirsTimer(new QTimer(this)) {
        // when we update the watched directories, the file system watcher can calculate whether there's new directories
        // to watch these
        QObject::connect(_watcher, &FileSystemWatcher::newDirectoriesToWatch, this, [this](const QDirSet& newDirs) {
            if (newDirs.empty()) {
                qDebug() << "No new directories to watch detected";
            } else {
                qCInfo(daemonCat) << "Discovered new directories to watch, integrating existing AppImages initially";

                initialSearchForAppImages(newDirs);

                // (re-)integrate all AppImages at once
                _worker->executeDeferredOperations();
            }
        });


        // whenever a formerly watched directory disappears, we want to clean the menu from entries pointing to AppImages
        // in this directory
        // a good example for this situation is a removable drive that has been unplugged from the computer
        QObject::connect(_watcher, &FileSystemWatcher::directoriesToWatchDisappeared, this, [](const QDirSet& disappearedDirs) {
             if (disappearedDirs.empty()) {
                 qDebug() << "No directories disappeared";
             } else {
                 qCInfo(daemonCat) << "Directories to watch disappeared, unintegrating AppImages formerly found in there";

                 if (!cleanUpOldDesktopIntegrationResources(true)) {
                     qCCritical(daemonCat) << "Error: Failed to clean up old desktop integration resources";
                 }
             }
         });


        // search directories to watch once initially
        // we *have* to do this even though we connect this signal above, as the first update occurs in the constructor
        // and we cannot connect signals before construction has finished for obvious reasons
        initialSearchForAppImages(_watcher->directories());

        // (re-)integrate all AppImages at once
        _worker->executeDeferredOperations();

        _updateWatchedDirsTimer->setInterval(UPDATE_WATCHED_DIRECTORIES_INTERVAL);
        connect(
            _updateWatchedDirsTimer, &QTimer::timeout, this,[this]() {
                _watcher->updateWatchedDirectories(this->watchedDirectories());
            }
        );
        _updateWatchedDirsTimer->start();


        connect(_watcher, &FileSystemWatcher::fileChanged, _worker, &Worker::scheduleForIntegration,
                Qt::QueuedConnection);
        connect(_watcher, &FileSystemWatcher::fileRemoved, _worker, &Worker::scheduleForUnintegration,
               Qt::QueuedConnection);
    }

    QDirSet Daemon::watchedDirectories() const {
        return daemonDirectoriesToWatch(_settings);
    }


    void Daemon::initialSearchForAppImages(const QDirSet& dirsToSearch) {
        // initial search for AppImages; if AppImages are found, they will be integrated, unless they already are
        qCInfo(daemonCat) << "Searching for existing AppImages";

        for (const auto& dir : dirsToSearch) {
            if (!dir.exists()) {
                qDebug() << "Directory " << dir.path() << " does not exist, skipping";
                continue;
            }

            qCInfo(daemonCat) << "Searching directory: " << dir.absolutePath();

            for (QDirIterator it(dir); it.hasNext();) {
                const auto& path = it.next();

                if (QFileInfo(path).isFile()) {
                    const auto appImageType = appimage_get_type(path.toStdString().c_str(), false);
                    const auto isAppImage = 0 < appImageType && appImageType <= 2;

                    if (isAppImage) {
                        // at application startup, we don't want to integrate AppImages that have been integrated already,
                        // as that it slows down very much
                        // the integration will be updated as soon as any of these AppImages is run with AppImageLauncher
                        qCInfo(daemonCat) << "Found AppImage: " << path;

                        if (!appimage_is_registered_in_system(path.toStdString().c_str())) {
                            qCInfo(daemonCat) << "AppImage is not integrated yet, integrating";
                            _worker->scheduleForIntegration(path);
                        } else if (!desktopFileHasBeenUpdatedSinceLastUpdate(path)) {
                            qCInfo(daemonCat) << "AppImage has been integrated already but needs to be reintegrated";
                            _worker->scheduleForIntegration(path);
                        } else {
                            qCInfo(daemonCat) << "AppImage integrated already, skipping";
                        }
                    }
                }
            }
        }
    }

    bool Daemon::startWatching() {
        return _watcher->startWatching();
    }

    void Daemon::slotStopWatching() {
        _watcher->stopWatching();
    }

}
