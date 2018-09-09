// system includes
#include <fstream>
#include <sstream>

extern "C" {
#include <glib.h>
}

// library includes
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QDebug>

// local headers
#include "shared.h"
#include "trashbin.h"
#include "translationmanager.h"
#include "Launcher.h"
#include "ui.h"

bool isDisplayAvailable() {
    auto screens = QGuiApplication::screens();
    return !screens.empty();
}

std::string getVersionString() {
    std::ostringstream version;
    version << "version " << APPIMAGELAUNCHER_VERSION << " "
            << "(git commit " << APPIMAGELAUNCHER_GIT_COMMIT << "), built on "
            << APPIMAGELAUNCHER_BUILD_DATE;
    return version.str();
}

void setApplicationVersion(const QCoreApplication *app) {
    auto version = getVersionString();
    app->setApplicationVersion(QString::fromStdString(version));
}

void printUsage(char **argv) {
    std::ostringstream usage;
    usage << QObject::tr("Usage: %1 [options] <path>").arg(argv[0]).toStdString() << std::endl
          << QObject::tr("Desktop integration helper for AppImages, for use by Linux distributions.").toStdString()
          << std::endl
          << std::endl
          << QObject::tr("Options:").toStdString() << std::endl
          << "  --appimagelauncher-help     " << QObject::tr("Display this help and exit").toStdString()
          << std::endl
          << "  --appimagelauncher-version  " << QObject::tr("Display version and exit").toStdString() << std::endl
          << std::endl
          << QObject::tr("Arguments:").toStdString() << std::endl
          << "  path                        " << QObject::tr("Path to AppImage (mandatory)").toStdString()
          << std::endl;

    std::cout << usage.str();
}

void printVersion() {
    std::cerr << "AppImageLauncher " << getVersionString() << std::endl;
}

void earlyArgumentsCheck(int argc, char **argv) {
    if (argc <= 1) {
        std::cerr << getVersionString() << std::endl;
        printUsage(argv);
        exit(1);
    }
}

std::vector<char *> parseArguments(int argc, char **argv) {
    std::vector<char *> appImageArgv;
    // search for --appimagelauncher-* arguments in args list
    for (int i = 1; i < argc; i++) {
        QString arg = argv[i];

        // reserved argument space
        const QString prefix = "--appimagelauncher-";

        if (arg.startsWith(prefix)) {
            if (arg == prefix + "help") {
                printVersion();
                printUsage(argv);
                exit(0);
            } else if (arg == prefix + "version") {
                printVersion();
                exit(0);
            } else if (arg == prefix + "cleanup") {
                // exit immediately after cleanup
                exit(0);
            } else {
                std::cerr << QObject::tr("Unknown AppImageLauncher option: %1").arg(arg).toStdString() << std::endl;
                exit(1);
            }
        } else {
            appImageArgv.emplace_back(argv[i]);
        }
    }
    return appImageArgv;
}

/**
 * Enable and start or disable and stop appimagelauncherd service
 */
void applyDaemonConfig() {
    auto config = getConfig();
    if (config->contains("enable_daemon") && config->value("enable_daemon").toBool()) {
        system("systemctl --user enable appimagelauncherd.service");
        system("systemctl --user start  appimagelauncherd.service");
    } else {
        system("systemctl --user disable appimagelauncherd.service");
        system("systemctl --user stop    appimagelauncherd.service");
    }
}

QMessageBox::StandardButton askToMoveFileIntoApplications(const QString &pathToAppImage,
                                                          const QString &integratedAppImagesDirPath) {
    auto rv = QMessageBox::warning(
            nullptr,
            QMessageBox::tr("Warning"),
            QMessageBox::tr(
                    "AppImage %1 has already been integrated, but it is not in the current integration "
                    "destination directory."
                    "\n\n"
                    "Do you want to move it into the new destination?"
                    "\n\n"
                    "Choosing No will run the AppImage once, and leave the AppImage in its current "
                    "directory."
                    "\n\n").arg(pathToAppImage) +
            // translate separately to share string with the other dialog
            QObject::tr("The directory the integrated AppImages are stored in is currently set to:\n"
                        "%1").arg(integratedAppImagesDirPath) + "\n",
            QMessageBox::Yes | QMessageBox::No
    );
    return rv;
}

int executeGuiApplication(int argc, char **argv) {
    // Create a fake argc value to avoid QApplication from modifying the arguments.
    int fakeArgc = 1;
    QApplication application(fakeArgc, argv);
    QApplication::setApplicationDisplayName("AppImageLauncher");
    setApplicationVersion(&application);

    TranslationManager translationManager(*QCoreApplication::instance());

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


    std::vector<char *> appImageArgv = parseArguments(argc, argv);

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
        QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("No such file or directory: %1").arg(pathToAppImage));
        return 1;
    } catch (const UnsuportedAppImageType &ex) {
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
            QMessageBox::critical(
                    nullptr,
                    QObject::tr("Error"),
                    QObject::tr("Unable to execute the AppImage: %1").arg(pathToAppImage));
            return 2;
        }
    }
    const auto pathToIntegratedAppImage = buildPathToIntegratedAppImage(pathToAppImage);

    // after checking whether the AppImage can/must be run without integrating it, we now check whether it actually
    // has been integrated already

    if (integrationManager.hasAlreadyBeenIntegrated(pathToAppImage)) {
        if (!integrationManager.isPlacedInTheDefaultAppsDir(pathToAppImage)) {
            auto response = askToMoveFileIntoApplications(pathToAppImage,
                                                          integrationManager.getIntegratedAppImagesDirPath());

            // if the user selects No, then continue as if the AppImage would not be in this directory
            if (response == QMessageBox::Yes) {
                integrationManager.removeAppImageIntegration(pathToAppImage);
                integrationManager.integrateAppImage(pathToAppImage);
                launcher.setAppImagePath(pathToIntegratedAppImage);
                launcher.executeAppImage();
            } else {
                integrationManager.updateAppImage(pathToAppImage);
                launcher.setAppImagePath(pathToAppImage);
                launcher.executeAppImage();
            }
        } else {
            integrationManager.updateAppImage(pathToAppImage);
            launcher.executeAppImage();
        }
    }


    ui.setLauncher(&launcher);
    ui.showIntegrationPage();

    return application.exec();
}

int executeCliApplication(int argc, char **argv) {
// Create a fake argc value to avoid QApplication from modifying the arguments.
    int fakeArgc = 1;
    QCoreApplication application(fakeArgc, argv);
    QCoreApplication::setApplicationName("AppImageLauncher");
    setApplicationVersion(&application);

    TranslationManager translationManager(*QCoreApplication::instance());

    // clean up old desktop files
    if (!cleanUpOldDesktopIntegrationResources())
        qWarning() << QObject::tr("Failed to clean up old desktop files");

    // clean up trash directory
    TrashBin trashBin;
    if (!trashBin.cleanUp())
        qWarning() << QObject::tr("Failed to clean up AppImage trash bin: %1").arg(trashBin.path());

    std::vector<char *> appImageArgv = parseArguments(argc, argv);

    // sanitize path
    auto pathToAppImage = QDir(QString(argv[1])).absolutePath();

    AppImageDesktopIntegrationManager integrationManager;
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
        qCritical() << QObject::tr("Not an AppImage: %1").arg(pathToAppImage);
        return 1;
    } catch (const AppImageFileNotExists &ex) {
        qCritical() << QObject::tr("No such file or directory: %1").arg(pathToAppImage);
        return 1;
    } catch (const UnsuportedAppImageType &ex) {
        qCritical() << QObject::tr("Not an AppImage: %1").arg(pathToAppImage);
        return 1;
    }

    if (launcher.isAppImageExecutable()) {
        try {
            launcher.executeAppImage();
            return 1;
        } catch (const std::runtime_error &ex) {
            qCritical() << QObject::tr("Unable to execute the AppImage: %1").arg(pathToAppImage);
            return 2;
        }
    } else {
        qWarning() << QObject::tr("AppImage execution failed, no enough permissions: %1").arg(pathToAppImage);
        return 2;
    }
}

int main(int argc, char **argv) {
    earlyArgumentsCheck(argc, argv);
    applyDaemonConfig();

    if (isDisplayAvailable())
        return executeGuiApplication(argc, argv);
    else
        return executeCliApplication(argc, argv);

}

