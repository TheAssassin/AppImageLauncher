// system includes
#include <fstream>
#include <iostream>
#include <sstream>
extern "C" {
    #include <sys/stat.h>
    #include <libgen.h>
    #include <unistd.h>
    #include <glib.h>
}

// library includes
#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QString>
#include <QTemporaryDir>
#include <QTextStream>
extern "C" {
    #include <appimage/appimage.h>
}

// local headers
#include "shared.h"
#include "trashbin.h"
#include "translationmanager.h"
#include "first-run.h"

// Runs an AppImage. Returns suitable exit code for main application.
int runAppImage(const QString& pathToAppImage, unsigned long argc, char** argv) {
    // needs to be converted to std::string to be able to use c_str()
    // when using QString and then .toStdString().c_str(), the std::string instance will be an rvalue, and the
    // pointer returned by c_str() will be invalid
    auto x = pathToAppImage.toStdString();
    auto fullPathToAppImage = QFileInfo(pathToAppImage).absoluteFilePath();

    auto type = appimage_get_type(fullPathToAppImage.toStdString().c_str(), false);
    if (type < 1 || type > 3) {
        displayError(QObject::tr("AppImageLauncher does not support type %1 AppImages at the moment.").arg(type));
        return 1;
    }

    // first of all, chmod +x the AppImage registerFile
    // not strictly necessary for AppImageLauncherFS, but if AppImageLauncher is going to be removed, the user will
    // be happy the registerFile is executable already
    if (!makeExecutable(fullPathToAppImage)) {
        displayError(QObject::tr("Could not make AppImage executable: %1").arg(fullPathToAppImage));
        return 1;
    }

    // make sure to restart service in case AppImageLauncher has been updated
//    if (!fsDaemonHasBeenRestartedSinceLastUpdate()) {
//        auto rv = system("systemctl --user restart appimagelauncherfs 2>&1 1>/dev/null");
//        if (rv != 0) {
//            displayError(QObject::tr("Failed to register AppImage in AppImageLauncherFS: error while trying to restart appimagelauncherfs.service after update"));
//            return 1;
//        }
//    }

    // make sure appimagelauncherfs service is running
    auto rv = system("systemctl --user enable appimagelauncherfs 2>&1 1>/dev/null");
    rv += system("systemctl --user start appimagelauncherfs 2>&1 1>/dev/null");

    if (rv != 0) {
        displayError(QObject::tr("Failed to register AppImage in AppImageLauncherFS: error while trying to start appimagelauncherfs.service"));
        return 1;
    }

    // suppress desktop integration script etc.
    setenv("DESKTOPINTEGRATION", "AppImageLauncher", true);

    // build path to AppImageLauncherFS endpoint
    QString pathToFSEndpoint = "/run/user/" + QString::number(getuid()) + "/appimagelauncherfs/";

    // resolve path to virtual file in map
    QString pathToVirtualAppImage;

    bool registrationWorked = false, pathResolvingWorked = false, openingMapFileWorked = false;

    // try to fetch ID of registered AppImage up to 10 times, with increasing timeouts
    for (useconds_t i = 1; i < 10; i++) {
        usleep(i * 100000);

        // register current AppImage
        if (!registrationWorked){
            QFile registerFile(pathToFSEndpoint + "/register");

            if (registerFile.open(QIODevice::Append)) {
                QTextStream stream(&registerFile);
                stream << pathToAppImage << endl;
            }

            registrationWorked = true;

            registerFile.close();
        }

        // reading line wise properly is [hard, impossible[ in Qt
        // using good ol' C++
        std::string mapFilePath = (pathToFSEndpoint + "/map").toStdString();

        std::ifstream mapFile(mapFilePath);

        if (!mapFile) {
            openingMapFileWorked = false;
            continue;
        }

        openingMapFileWorked = true;

        std::string currentLine;
        while (std::getline(mapFile, currentLine)) {
            if (currentLine.find(pathToAppImage.toStdString()) != std::string::npos) {
                auto virtualFileName = currentLine.substr(0, currentLine.find(" -> "));
                pathToVirtualAppImage = pathToFSEndpoint + "/" + QString::fromStdString(virtualFileName);
                break;
            }
        }

        mapFile.close();

        if (!pathToVirtualAppImage.isEmpty()) {
            pathResolvingWorked = true;
            break;
        }
    }

    // error message handling
    if (pathToVirtualAppImage.isEmpty()) {
        QString errorMessage;

        if (!registrationWorked) {
            errorMessage = QObject::tr("Failed to register AppImage in AppImageLauncherFS: failed to register AppImage path %1").arg(pathToAppImage);
        } else if (!openingMapFileWorked) {
            errorMessage = QObject::tr("Failed to register AppImage in AppImageLauncherFS: could not open map file");
        } else if (!pathResolvingWorked) {
            errorMessage = QObject::tr("Failed to register AppImage in AppImageLauncherFS: could not find virtual file for AppImage");
        } else {
            // this should _never_ be reachable
            errorMessage = QObject::tr("Failed to register AppImage in AppImageLauncherFS: unknown failure");
        }

        displayError(errorMessage);

        return 1;
    }

    // need a char pointer instead of a const one, therefore can't use .c_str()
    std::vector<char> argv0Buffer(pathToAppImage.toStdString().size() + 1, '\0');
    strcpy(argv0Buffer.data(), pathToAppImage.toStdString().c_str());

    std::vector<char*> args;

    args.push_back(argv0Buffer.data());

    // copy arguments
    for (unsigned long i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    // args need to be null terminated
    args.push_back(nullptr);

    execv(pathToVirtualAppImage.toStdString().c_str(), args.data());

    const auto& error = errno;
    std::cerr << QObject::tr("execv() failed: %1").arg(strerror(error)).toStdString() << std::endl;
    return 1;
}

// factory method to build and return a suitable Qt application instance
// it remembers a previously created instance, and will return it if available
// otherwise a new one is created and configure
// caution: cannot use <widget>.exec() any more, instead call <widget>.show() and use QApplication::exec()
QCoreApplication* getApp(char** argv) {
    if (QCoreApplication::instance() != nullptr)
        return QCoreApplication::instance();

    // build application version string
    std::string version;
    {
        std::ostringstream oss;
        oss << "version " << APPIMAGELAUNCHER_VERSION << " "
            << "(git commit " << APPIMAGELAUNCHER_GIT_COMMIT << "), built on "
            << APPIMAGELAUNCHER_BUILD_DATE;
        version = oss.str();
    }

    QCoreApplication* app;

    // need to pass rvalue, hence defining a variable
    int* fakeArgc = new int{1};

    static char** fakeArgv = new char*{strdup(argv[0])};

    if (isHeadless()) {
        app = new QCoreApplication(*fakeArgc, fakeArgv);
    } else {
        auto uiApp = new QApplication(*fakeArgc, fakeArgv);
        QApplication::setApplicationDisplayName("AppImageLauncher");

        // this doesn't seem to have any effect... but it doesn't hurt either
        uiApp->setWindowIcon(QIcon(":/AppImageLauncher.svg"));

        app = uiApp;
    }

    QCoreApplication::setApplicationName("AppImageLauncher");
    QCoreApplication::setApplicationVersion(QString::fromStdString(version));

    return app;
}

int main(int argc, char** argv) {
    // create a suitable application object (either graphical (QApplication) or headless (QCoreApplication))
    // Use a fake argc value to avoid QApplication from modifying the arguments
    QCoreApplication* app = getApp(argv);

    // install translations
    TranslationManager translationManager(*app);

    // clean up old desktop files
    if (!cleanUpOldDesktopIntegrationResources()) {
        displayError(QObject::tr("Failed to clean up old desktop files"));
        return 1;
    }

    // clean up trash directory
    {
        TrashBin bin;
        if (!bin.cleanUp()) {
            displayError(QObject::tr("Failed to clean up AppImage trash bin: %1").arg(bin.path()));
        }
    }

    std::ostringstream usage;
    usage << QObject::tr("Usage: %1 [options] <path>").arg(argv[0]).toStdString() << std::endl
          << QObject::tr("Desktop integration helper for AppImages, for use by Linux distributions.").toStdString() << std::endl
          << std::endl
          << QObject::tr("Options:").toStdString() << std::endl
          << "  --appimagelauncher-help     " << QObject::tr("Display this help and exit").toStdString() << std::endl
          << "  --appimagelauncher-version  " << QObject::tr("Display version and exit").toStdString() << std::endl
          << std::endl
          << QObject::tr("Arguments:").toStdString() << std::endl
          << "  path                        " << QObject::tr("Path to AppImage (mandatory)").toStdString() << std::endl;

    auto displayVersion = [&app]() {
        std::cerr << "AppImageLauncher " << app->applicationVersion().toStdString() << std::endl;
    };

    // display usage and exit if path to AppImage is missing
    if (argc <= 1) {
        displayVersion();
        std::cerr << std::endl;
        std::cerr << usage.str();
        return 1;
    }

    std::vector<char*> appImageArgv;
    // search for --appimagelauncher-* arguments in args list
    for (int i = 1; i < argc; i++) {
        QString arg = argv[i];

        // reserved argument space
        const QString prefix = "--appimagelauncher-";

        if (arg.startsWith(prefix)) {
            if (arg == prefix + "help") {
                displayVersion();
                std::cerr << std::endl;
                std::cerr << usage.str();
                return 0;
            } else if (arg == prefix + "version") {
                displayVersion();
                return 0;
            } else if (arg == prefix + "cleanup") {
                // exit immediately after cleanup
                return 0;
            } else {
                std::cerr << QObject::tr("Unknown AppImageLauncher option: %1").arg(arg).toStdString() << std::endl;
                return 1;
            }
        } else {
            appImageArgv.emplace_back(argv[i]);
        }
    }

    // sanitize path
    auto pathToAppImage = QDir(QString(argv[1])).absolutePath();

    if (!QFile(pathToAppImage).exists()) {
        displayError(QObject::tr("Error: no such file or directory: %1").arg(pathToAppImage));
        return 1;
    }

    // if the users wishes to disable AppImageLauncher, we just run the AppImage as-ish
    if (getenv("APPIMAGELAUNCHER_DISABLE") != nullptr) {
        return runAppImage(pathToAppImage, appImageArgv.size(), appImageArgv.data());
    }

    const auto type = appimage_get_type(pathToAppImage.toStdString().c_str(), false);

    if (type <= 0 || type > 2) {
        displayError(QObject::tr("Not an AppImage: %1").arg(pathToAppImage));
        return 1;
    }

    // type 2 specific checks
    if (type == 2) {
        // check parameters
        {
            for (int i = 0; i < argc; i++) {
                QString arg = argv[i];

                // reserved argument space
                const QString prefix = "--appimage-";

                if (arg.startsWith(prefix)) {
                    // don't annoy users who try to mount or extract AppImages
                    if (arg == prefix + "mount" || arg == prefix + "extract" || arg == prefix + "updateinformation") {
                        return runAppImage(pathToAppImage, appImageArgv.size(), appImageArgv.data());
                    }
                }
            }
        }
    }

    // enable and start/disable and stop appimagelauncherd service
    auto config = getConfig();

    // assumes defaults if config doesn't exist or lacks the related key(s)
    if (config == nullptr || !config->contains("AppImageLauncher/enable_daemon") || config->value("AppImageLauncher/enable_daemon").toBool()) {
        system("systemctl --user enable appimagelauncherd.service");
        system("systemctl --user start  appimagelauncherd.service");
    } else {
        system("systemctl --user disable appimagelauncherd.service");
        system("systemctl --user stop    appimagelauncherd.service");
    }

    // beyond the next block, the code requires a UI
    // as we don't want to offer integration over a headless connection, we just run the AppImage
    if (isHeadless()) {
        return runAppImage(pathToAppImage, appImageArgv.size(), appImageArgv.data());
    }

    // if config doesn't exist, create a default one
    if (config == nullptr) {
        showFirstRunDialog();
        config = getConfig();
    }

    if (config == nullptr) {
        displayError("Could not read config file");
    }

    // if the user opted out of the "ask move" thing, we can just run the AppImage
    if (config->contains("AppImageLauncher/ask_to_move") && !config->value("AppImageLauncher/ask_to_move").toBool()) {
        return runAppImage(pathToAppImage, appImageArgv.size(), appImageArgv.data());
    }

    // check for X-AppImage-Integrate=false
    auto shallNotBeIntegrated = appimage_shall_not_be_integrated(pathToAppImage.toStdString().c_str());
    if (shallNotBeIntegrated < 0)
        std::cerr << "AppImageLauncher error: appimage_shall_not_be_integrated() failed (returned " << shallNotBeIntegrated << ")" << std::endl;
    else if (shallNotBeIntegrated > 0)
        return runAppImage(pathToAppImage, appImageArgv.size(), appImageArgv.data());

    // AppImages in AppImages are not supposed to be integrated
    if (pathToAppImage.startsWith("/tmp/.mount_"))
        return runAppImage(pathToAppImage, appImageArgv.size(), appImageArgv.data());

    // ignore terminal apps (fixes #2)
    auto isTerminalApp = appimage_is_terminal_app(pathToAppImage.toStdString().c_str());
    if (isTerminalApp < 0)
        std::cerr << "AppImageLauncher error: appimage_is_terminal_app() failed (returned " << isTerminalApp << ")" << std::endl;
    else if (isTerminalApp > 0)
        return runAppImage(pathToAppImage, appImageArgv.size(), appImageArgv.data());

    // AppImages in AppImages are not supposed to be integrated
    if (pathToAppImage.startsWith("/tmp/.mount_"))
        return runAppImage(pathToAppImage, appImageArgv.size(), appImageArgv.data());

    const auto pathToIntegratedAppImage = buildPathToIntegratedAppImage(pathToAppImage);

    auto integrateAndRunAppImage = [&pathToAppImage, &pathToIntegratedAppImage, &appImageArgv]() {
        // check whether integration was successful
        auto rv = integrateAppImage(pathToAppImage, pathToIntegratedAppImage);

        // make sure the icons in the launcher are refreshed
        if (!updateDesktopDatabaseAndIconCaches())
            return 1;

        if (rv == INTEGRATION_FAILED) {
            return 1;
        } else if (rv == INTEGRATION_ABORTED) {
            return runAppImage(pathToAppImage, appImageArgv.size(), appImageArgv.data());
        } else {
            return runAppImage(pathToIntegratedAppImage, appImageArgv.size(), appImageArgv.data());
        }
    };

    // after checking whether the AppImage can/must be run without integrating it, we now check whether it actually
    // has been integrated already
    if (hasAlreadyBeenIntegrated(pathToAppImage)) {
        auto updateAndRunAppImage = [&pathToAppImage, &appImageArgv]() {
            // in case there was an update of AppImageLauncher, we should should also update the desktop database
            // and icon caches
            if (!desktopFileHasBeenUpdatedSinceLastUpdate(pathToAppImage)) {
                if (!updateDesktopFileAndIcons(pathToAppImage))
                    return 1;

                // make sure the icons in the launcher are refreshed after updating the desktop file
                if (!updateDesktopDatabaseAndIconCaches())
                    return 1;
            }
            return runAppImage(pathToAppImage, appImageArgv.size(), appImageArgv.data());
        };

        bool needToAskAboutMoving = !isInDirectory(pathToAppImage, integratedAppImagesDestination().path());

        // not so fast: even if it's not in the main integration directory, there's more viable locations where
        // AppImages may reside just fine
        if (needToAskAboutMoving) {
            for (const auto& additionalLocation : additionalAppImagesLocations()) {
                if (isInDirectory(pathToAppImage, additionalLocation)) {
                    needToAskAboutMoving = false;
                }
            }
        }

        if (needToAskAboutMoving) {
            auto* messageBox = new QMessageBox(
                QMessageBox::Warning,
                QMessageBox::tr("Warning"),
                QMessageBox::tr("AppImage %1 has already been integrated, but it is not in the current integration "
                                "destination directory."
                                "\n\n"
                                "Do you want to move it into the new destination?"
                                "\n\n"
                                "Choosing No will run the AppImage once, and leave the AppImage in its current "
                                "directory."
                                "\n\n").arg(pathToAppImage) +
                                // translate separately to share string with the other dialog
                                QObject::tr("The directory the integrated AppImages are stored in is currently set to:\n"
                                            "%1").arg(integratedAppImagesDestination().path()) + "\n",
                QMessageBox::Yes | QMessageBox::No
            );

            messageBox->setDefaultButton(QMessageBox::Yes);
            messageBox->show();

            QApplication::exec();

            // if the user selects No, then continue as if the AppImage would not be in this directory
            if (messageBox->clickedButton() == messageBox->button(QMessageBox::Yes)) {
                // unregister AppImage, move, and re-integrate
                if (appimage_unregister_in_system(pathToAppImage.toStdString().c_str(), false) != 0) {
                    displayError(QMessageBox::tr("Failed to unregister AppImage before re-integrating it"));
                    return 1;
                }

                return integrateAndRunAppImage();
            } else {
                return updateAndRunAppImage();
            }
        } else {
            return updateAndRunAppImage();
        }
    }

    std::ostringstream explanationStrm;
    explanationStrm << QObject::tr("Integrating it will move the AppImage into a predefined location, "
                       "and include it in your application launcher.").toStdString() << std::endl
                    << std::endl
                    << QObject::tr("To remove or update the AppImage, please use the context menu of the "
                       "application icon in your task bar or launcher.").toStdString() << std::endl
                    << std::endl
                    << QObject::tr("The directory the integrated AppImages are stored in is currently "
                                        "set to:").toStdString() << std::endl
                    << integratedAppImagesDestination().path().toStdString() << std::endl;

    auto explanation = explanationStrm.str();

    std::ostringstream messageStrm;
    messageStrm << QObject::tr("%1 has not been integrated into your system.").arg(pathToAppImage).toStdString() << "\n\n"
                << QObject::tr(explanation.c_str()).toStdString();

    auto* messageBox = new QMessageBox(
        QMessageBox::Question,
        QObject::tr("Desktop Integration"),
        QString::fromStdString(messageStrm.str())
    );

    auto* okButton = messageBox->addButton(QObject::tr("Integrate and run"), QMessageBox::AcceptRole);
    auto* runOnceButton = messageBox->addButton(QObject::tr("Run once"), QMessageBox::ApplyRole);

    // *whyever* Qt somehow needs a button with "RejectRole" or the X button won't close the current window...
    auto* cancelButton = messageBox->addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    // ... but it is fine to hide that button after creating it, so it's not displayed
    cancelButton->hide();

    messageBox->setDefaultButton(QMessageBox::Ok);

    // cannot use messageBox.exec(), will produce SEGFAULTS as QCoreApplications can't show message boxes
    messageBox->show();

    // don't need to cast around, exec() is a static method anyway, and QApplication is a singleton
    QApplication::exec();

    const auto* clickedButton = messageBox->clickedButton();

    if (clickedButton == okButton) {
        return integrateAndRunAppImage();
    } else if (clickedButton == runOnceButton) {
        return runAppImage(pathToAppImage, appImageArgv.size(), appImageArgv.data());
    } else if (clickedButton == cancelButton) {
        return 0;
    }

    // _should_ be unreachable
    return 1;
}

