// system includes
#include <deque>
#include <iostream>
#include <set>
#include <sstream>
#include <sys/stat.h>

// library includes
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDirIterator>
#include <QTimer>
#include <appimage/appimage.h>

// local includes
#include "shared.h"
#include "filesystemwatcher.h"
#include "worker.h"

#define UPDATE_WATCHED_DIRECTORIES_INTERVAL 30 * 1000

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

void initialSearchForAppImages(const QDirSet& dirsToSearch, Worker& worker) {
    // initial search for AppImages; if AppImages are found, they will be integrated, unless they already are
    std::cout << "Searching for existing AppImages" << std::endl;

    for (const auto& dir : dirsToSearch) {
        std::cout << "Searching directory: " << dir.absolutePath().toStdString() << std::endl;

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
}

int main(int argc, char* argv[]) {
    // make sure shared won't try to use the UI
    setenv("_FORCE_HEADLESS", "1", 1);

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QObject::tr(
            "Tracks AppImages in applications directories (user's, system and other ones). "
            "Automatically integrates AppImages moved into those directories and unintegrates ones removed from them."
        )
    );

    QCommandLineOption listWatchedDirectoriesOption(
        "list-watched-directories",
        QObject::tr("Lists directories watched by this daemon and exit")
    );

    if (!parser.addOption(listWatchedDirectoriesOption)) {
        throw std::runtime_error("could not add Qt command line option for some reason");
    }

    QCoreApplication app(argc, argv);

    {
        std::ostringstream version;
        version << "version " << APPIMAGELAUNCHER_VERSION << " "
                << "(git commit " << APPIMAGELAUNCHER_GIT_COMMIT << "), built on "
                << APPIMAGELAUNCHER_BUILD_DATE;
        QCoreApplication::setApplicationVersion(QString::fromStdString(version.str()));
    }

    // parse arguments
    parser.process(app);

    // load config file
    const auto config = getConfig();

    const auto listWatchedDirectories = parser.isSet(listWatchedDirectoriesOption);

    QDirSet watchedDirectories = daemonDirectoriesToWatch(config);

    // this option is for debugging the
    if (listWatchedDirectories) {
        for (const auto& watchedDir : watchedDirectories) {
            std::cout << watchedDir.absolutePath().toStdString() << std::endl;
        }
        return 0;
    }

    // time to create the watcher object
    FileSystemWatcher watcher(watchedDirectories);

    // create a daemon worker instance
    // it is used to integrate all AppImages initially, and to integrate files found via inotify
    Worker worker;

    // we we update the watched directories, the file system watcher can calculate whether there's new directories
    // to watch
    // these
    QObject::connect(&watcher, &FileSystemWatcher::newDirectoriesToWatch, &app, [&worker](const QDirSet& newDirs) {
        if (newDirs.empty()) {
            qDebug() << "No new directories to watch detected";
        } else {
            std::cout << "Discovered new directories to watch, integrating existing AppImages initially" << std::endl;

            initialSearchForAppImages(newDirs, worker);

            // (re-)integrate all AppImages at once
            worker.executeDeferredOperations();
        }
    });

    // whenever a formerly watched directory disappears, we want to clean the menu from entries pointing to AppImages
    // in this directory
    // a good example for this situation is a removable drive that has been unplugged from the computer
    QObject::connect(&watcher, &FileSystemWatcher::directoriesToWatchDisappeared, &app,
        [](const QDirSet& disappearedDirs) {

        if (disappearedDirs.empty()) {
            qDebug() << "No directories disappeared";
        } else {
            std::cout << "Directories to watch disappeared, unintegrating AppImages formerly found in there"
                      << std::endl;

            if (!cleanUpOldDesktopIntegrationResources(true)) {
                std::cerr << "Error: Failed to clean up old desktop integration resources" << std::endl;
            }
        }
    });

    // search directories to watch once initially
    // we *have* to do this even though we connect this signal above, as the first update occurs in the constructor
    // and we cannot connect signals before construction has finished for obvious reasons
    initialSearchForAppImages(watcher.directories(), worker);

    // (re-)integrate all AppImages at once
    worker.executeDeferredOperations();

    // we regularly want to update
    {
        auto* timer = new QTimer(&app);
        timer->setInterval(UPDATE_WATCHED_DIRECTORIES_INTERVAL);
        QTimer::connect(
            timer, &QTimer::timeout, &app,[&watcher]() {
                watcher.updateWatchedDirectories(daemonDirectoriesToWatch());
            }
        );
        timer->start();
    }

    // after (re-)integrating all AppImages, clean up old desktop integration resources before start
    if (!cleanUpOldDesktopIntegrationResources()) {
        std::cout << "Failed to clean up old desktop integration resources" << std::endl;
    }

    std::cout << "Watching directories: ";
    for (const auto& dir : watcher.directories()) {
        std::cout << dir.absolutePath().toStdString().c_str() << " ";
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

    QObject::connect(&app, &QCoreApplication::aboutToQuit, &watcher, &FileSystemWatcher::stopWatching);

    auto* binaryUpdatesMonitor = setupBinaryUpdatesMonitor(argv);
    binaryUpdatesMonitor->start();

    return QCoreApplication::exec();
}
