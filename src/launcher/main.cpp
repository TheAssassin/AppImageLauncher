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
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QRegularExpression>
#include <QDebug>

extern "C" {
    #include <appimage/appimage.h>
    #include <xdg-basedir.h>
}

// local headers
#include "shared.h"
#include "trashbin.h"
#include "translationmanager.h"
#include "Launcher.h"
#include "ui.h"

int main(int argc, char **argv) {
    // Create a fake argc value to avoid QApplication from modifying the arguments.
    int fakeArgc = 1;
    QApplication app(fakeArgc, argv);
    app.setApplicationDisplayName("AppImageLauncher");

    std::ostringstream version;
    version << "version " << APPIMAGELAUNCHER_VERSION << " "
            << "(git commit " << APPIMAGELAUNCHER_GIT_COMMIT << "), built on "
            << APPIMAGELAUNCHER_BUILD_DATE;
    app.setApplicationVersion(QString::fromStdString(version.str()));

    // install translations
    TranslationManager translationManager(app);

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
        std::cerr << "AppImageLauncher " << app.applicationVersion().toStdString() << std::endl;
    };

    // display usage and exit if path to AppImage is missing
    if (argc <= 1) {
        displayVersion();
        std::cerr << std::endl;
        std::cerr << usage.str();
        return 1;
    }

    // clean up old desktop files
    if (!cleanUpOldDesktopIntegrationResources()) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Error"),
            QObject::tr("Failed to clean up old desktop files")
        );
    }

    // clean up trash directory
    TrashBin trashBin;
    if (!trashBin.cleanUp()) {
        QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("Failed to clean up AppImage trash bin: %1").arg(trashBin.path())
        );
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

    AppImageDesktopIntegrationManager integrationManager;
    UI ui;
    Launcher launcher;
    launcher.setAppImagePath(pathToAppImage);
    launcher.setArgs(appImageArgv);
    launcher.setIntegrationManager(&integrationManager);
    launcher.setTrashBin(&trashBin);
    try {
        launcher.inspectAppImageFile();
    } catch (const AppImageFilePathNotSet &ex) {
        qCritical() << "Missing AppImagePath in Launcher class. I wasn't initialized properly.";
        return 1;
    } catch (const InvalidAppImageFile &ex) {
        QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("Not an AppImage: %1").arg(pathToAppImage));
        return 1;
    } catch (const AppImageFileNotExists &ex) {
        std::cout << QObject::tr("Error: no such file or directory: %1").arg(pathToAppImage).toStdString() << std::endl;
        return 1;
    }

    const auto type = appimage_get_type(pathToAppImage.toStdString().c_str(), false);

    if (type <= 0 || type > 2) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Error"),
            QObject::tr("Not an AppImage: %1").arg(pathToAppImage));
        return 1;
    }

    if (launcher.shouldBeIgnored()) {
        try {
            launcher.executeAppImage();
            return 0;
        } catch (const std::runtime_error &ex) {
            qCritical() << QObject::tr("Unable to execute the AppImage: %1").arg(ex.what());
            return 2;
        }
    }

    // enable and start/disable and stop appimagelauncherd service
    auto config = getConfig();
    if (config->contains("enable_daemon") && config->value("enable_daemon").toBool()) {
        system("systemctl --user enable appimagelauncherd.service");
        system("systemctl --user start  appimagelauncherd.service");
    } else {
        system("systemctl --user disable appimagelauncherd.service");
        system("systemctl --user stop    appimagelauncherd.service");
    }

    try {
        // check for X-AppImage-Integrate=false
        if (appimage_shall_not_be_integrated(pathToAppImage.toStdString().c_str()))
            launcher.executeAppImage();

        // AppImages in AppImages are not supposed to be integrated
        if (pathToAppImage.startsWith("/tmp/.mount_"))
            launcher.executeAppImage();

        // ignore terminal apps (fixes #2)
        if (appimage_is_terminal_app(pathToAppImage.toStdString().c_str()))
            launcher.executeAppImage();

        // AppImages in AppImages are not supposed to be integrated
        if (pathToAppImage.startsWith("/tmp/.mount_"))
            launcher.executeAppImage();
    } catch (const ExecutionFailed&) {
        qCritical() << QObject::tr("Failed to execute AppImage: %1").arg(pathToAppImage);
        return 1;
    }

    const auto pathToIntegratedAppImage = buildPathToIntegratedAppImage(pathToAppImage);

    // after checking whether the AppImage can/must be run without integrating it, we now check whether it actually
    // has been integrated already

    if (integrationManager.hasAlreadyBeenIntegrated(pathToAppImage)) {
        if (!isInDirectory(pathToAppImage, integratedAppImagesDestination().path())) {
            auto rv = QMessageBox::warning(
                nullptr,
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

            // if the user selects No, then continue as if the AppImage would not be in this directory
            if (rv == QMessageBox::Yes) {
                // unregister AppImage, move, and re-integrate
                if (appimage_unregister_in_system(pathToAppImage.toStdString().c_str(), false) != 0) {
                    QMessageBox::critical(
                        nullptr,
                        QMessageBox::tr("Error"),
                        QMessageBox::tr("Failed to unregister AppImage before re-integrating it")
                    );
                    return 1;
                }
                integrationManager.integrateAppImage(pathToAppImage);
                launcher.setAppImagePath(pathToIntegratedAppImage);

                try {
                    launcher.executeAppImage();
                } catch (const ExecutionFailed&) {
                    qCritical() << QObject::tr("Failed to execute AppImage: %1").arg(pathToAppImage);
                    return 1;
                }
                return 0;
            } else {
                integrationManager.updateAppImage(pathToAppImage);
                launcher.setAppImagePath(pathToAppImage);

                try {
                    launcher.executeAppImage();
                } catch (const ExecutionFailed&) {
                    qCritical() << QObject::tr("Failed to execute AppImage: %1").arg(pathToAppImage);
                    return 1;
                }
            }
        } else {
            integrationManager.updateAppImage(pathToAppImage);

            try {
                launcher.executeAppImage();
            } catch (const ExecutionFailed&) {
                qCritical() << QObject::tr("Failed to execute AppImage: %1").arg(pathToAppImage);
                return 1;
            }
        }
    }

    ui.setLauncher(&launcher);
    ui.showIntegrationPage();

    return app.exec();
}

