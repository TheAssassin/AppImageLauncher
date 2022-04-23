// system includes
#include <deque>
#include <iostream>
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
#include "daemon.h"

using namespace appimagelauncher::daemon;

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

    auto* daemon = new Daemon(&app);

    // this option is for debugging the
    if (parser.isSet(listWatchedDirectoriesOption)) {
        for (const auto& watchedDir : daemon->watchedDirectories()) {
            std::cout << watchedDir.absolutePath().toStdString() << std::endl;
        }
        return 0;
    }

    // after (re-)integrating all AppImages, clean up old desktop integration resources before start
    if (!cleanUpOldDesktopIntegrationResources()) {
        std::cout << "Failed to clean up old desktop integration resources" << std::endl;
    }

    auto watchedDirectoriesStringList = [daemon]() {
        QStringList rv;
        for (const auto& dir : daemon->watchedDirectories()) {
            rv << dir.path();
        }
        return rv;
    }();

    qInfo() << "Watching directories:" << watchedDirectoriesStringList;

    if (!daemon->startWatching()) {
        std::cerr << "Could not start watching directories" << std::endl;
        return 1;
    }

    QCoreApplication::connect(&app, &QCoreApplication::aboutToQuit, daemon, &Daemon::slotStopWatching);

    auto* binaryUpdatesMonitor = setupBinaryUpdatesMonitor(argv);
    binaryUpdatesMonitor->start();

    return QCoreApplication::exec();
}
