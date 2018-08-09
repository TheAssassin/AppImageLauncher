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
#include <QString>
#include <QTemporaryDir>
extern "C" {
    #include <appimage/appimage.h>
    #include <xdg-basedir.h>
}

// local headers
#include "shared.h"
#include "trashbin.h"
#include "translationmanager.h"

// Runs an AppImage. Returns suitable exit code for main application.
int runAppImage(const QString& pathToAppImage, int argc, char** argv) {
    // needs to be converted to std::string to be able to use c_str()
    // when using QString and then .toStdString().c_str(), the std::string instance will be an rvalue, and the
    // pointer returned by c_str() will be invalid
    auto x = pathToAppImage.toStdString();
    auto fullPathToAppImage = QFileInfo(pathToAppImage).absoluteFilePath();

    auto type = appimage_get_type(fullPathToAppImage.toStdString().c_str(), false);
    if (type < 1 || type > 3) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Error"),
            QObject::tr("AppImageLauncher does not support type %1 AppImages at the moment.").arg(type)
        );
        return 1;
    }

    // first of all, chmod +x the AppImage file, otherwise execv() will complain
    if (!makeExecutable(fullPathToAppImage)) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Error"),
            QObject::tr("Could not make AppImage executable: %1").arg(fullPathToAppImage)
        );
        return 1;
    }

    // build path to AppImage runtime
    // as it might error, check before fork()ing to be able to display an error message beforehand
    auto exeDir = QFileInfo(QFile("/proc/self/exe").symLinkTarget()).absoluteDir().absolutePath();

    if (type == 1) {
        auto size = appimage_get_elf_size(fullPathToAppImage.toStdString().c_str());

        QFile appImage(QString::fromStdString(fullPathToAppImage.toStdString()));

        if (!appImage.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("Failed to open AppImage for reading: %1").arg(pathToAppImage)
            );
            return 1;
        }

        // copy AppImage to temp dir
        QTemporaryDir tempDir("/tmp/AppImageLauncher-type1-XXXXXX");

        if (!tempDir.isValid()) {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("Failed to create temporary directory")
            );
            return 1;
        }

        tempDir.setAutoRemove(true);

        // copy AppImage to temporary directory
        auto tempAppImagePath = QDir(tempDir.path()).absoluteFilePath(QFileInfo(appImage).fileName());

        if (!appImage.copy(tempAppImagePath)) {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("Failed to create temporary copy of type 1 AppImage")
            );
            return 1;
        }

        QFile tempAppImage(tempAppImagePath);

        if (!tempAppImage.open(QFile::ReadWrite)) {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("Failed to open temporary AppImage copy for writing")
            );
            return 1;
        }

        // nuke magic bytes
        if (!tempAppImage.seek(8)) {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("Failed to remove magic bytes from temporary AppImage copy")
            );
            return 1;
        }
        if (tempAppImage.write(QByteArray(3, '\0')) != 3) {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("Failed to remove magic bytes from temporary AppImage copy")
            );
            return 1;
        }

        auto tempAppImageFileName = tempAppImage.fileName();

        // actually _write_ changes
        tempAppImage.close();

        makeExecutable(tempAppImageFileName);

        // need a char pointer instead of a const one, therefore can't use .c_str()
        std::vector<char> argv0Buffer(tempAppImageFileName.size() + 1, '\0');
        strcpy(argv0Buffer.data(), tempAppImageFileName.toStdString().c_str());

        std::vector<char*> args;

        args.push_back(argv0Buffer.data());

        // copy arguments
        // starting at index 2, as the first argument is supposed to be the path to an AppImage
        // all the other arguments can simply be copied
        for (int i = 2; i < argc; i++) {
            args.push_back(argv[i]);
        }

        // args need to be null terminated
        args.push_back(nullptr);

        execv(tempAppImageFileName.toStdString().c_str(), args.data());

        const auto& error = errno;
        std::cout << QObject::tr("execv() failed: %1", "error message").arg(strerror(error)).toStdString() << std::endl;
    } else if (type == 2) {
        // use external runtime _without_ magic bytes to run the AppImage
        // the original idea was to use /lib64/ld-linux-x86_64.so to run AppImages, but it did complain about some
        // violated ELF data, and refused to run the AppImage
        // alternatively, the AppImage would have to be mounted by this application, and AppRun would need to be called
        // however, this requires some process management (e.g., killing all processes inside the AppImage and also
        // the FUSE "mount" process, when this application is killed...)
        setenv("TARGET_APPIMAGE", fullPathToAppImage.toStdString().c_str(), true);

        // suppress desktop integration script
        setenv("DESKTOPINTEGRATION", "AppImageLauncher", true);

        // first attempt: find runtime in expected installation directory
        auto pathToRuntime = exeDir.toStdString() + "/../lib/appimagelauncher/runtime";

        // next method: find runtime in expected build location
        if (!QFile(QString::fromStdString(pathToRuntime)).exists()) {
            pathToRuntime = exeDir.toStdString() + "/../lib/AppImageKit/src/runtime";
        }

        // if it can't be found in either location, display error and exit
        if (!QFile(QString::fromStdString(pathToRuntime)).exists()) {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("runtime not found: no such file or directory: %1").arg(QString::fromStdString(pathToRuntime))
            );
            return 1;
        }

        // need a char pointer instead of a const one, therefore can't use .c_str()
        std::vector<char> argv0Buffer(pathToRuntime.size() + 1, '\0');
        strcpy(argv0Buffer.data(), pathToRuntime.c_str());

        std::vector<char*> args;

        args.push_back(argv0Buffer.data());

        // copy arguments
        // starting at index 2, as the first argument is supposed to be the path to an AppImage
        // all the other arguments can simply be copied
        for (int i = 2; i < argc; i++) {
            args.push_back(argv[i]);
        }

        // args need to be null terminated
        args.push_back(nullptr);

        execv(pathToRuntime.c_str(), args.data());

        const auto& error = errno;
        std::cout << QObject::tr("execv() failed: %1").arg(strerror(error)).toStdString() << std::endl;
    }
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
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
    {
        TrashBin bin;
        if (!bin.cleanUp()) {
            QMessageBox::critical(
                nullptr,
                QObject::tr("Error"),
                QObject::tr("Failed to clean up AppImage trash bin: %1").arg(bin.path())
            );
        }
    }

    // search for --appimagelauncher-* arguments in args list
    for (int i = 0; i < argc; i++) {
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
        }
    }

    // sanitize path
    auto pathToAppImage = QDir(QString(argv[1])).absolutePath();

    if (!QFile(pathToAppImage).exists()) {
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
                    if (arg == prefix + "mount" || arg == prefix + "extract") {
                        return runAppImage(pathToAppImage, argc, argv);
                    }
                }
            }
        }
    }

    // check for X-AppImage-Integrate=false
    if (appimage_shall_not_be_integrated(pathToAppImage.toStdString().c_str()))
        return runAppImage(pathToAppImage, argc, argv);

    // AppImages in AppImages are not supposed to be integrated
    if (pathToAppImage.startsWith("/tmp/.mount_"))
        return runAppImage(pathToAppImage, argc, argv);

    // ignore terminal apps (fixes #2)
    if (appimage_is_terminal_app(pathToAppImage.toStdString().c_str()))
        return runAppImage(pathToAppImage, argc, argv);

    // AppImages in AppImages are not supposed to be integrated
    if (pathToAppImage.startsWith("/tmp/.mount_"))
        return runAppImage(pathToAppImage, argc, argv);

    const auto pathToIntegratedAppImage = buildPathToIntegratedAppImage(pathToAppImage);

    auto integrateAndRunAppImage = [&pathToAppImage, &pathToIntegratedAppImage, argc, &argv]() {
        // check whether integration was successful
        auto rv = integrateAppImage(pathToAppImage, pathToIntegratedAppImage);

        // make sure the icons in the launcher are refreshed
        if (!updateDesktopDatabaseAndIconCaches())
            return 1;

        if (rv == INTEGRATION_FAILED) {
            return 1;
        } else if (rv == INTEGRATION_ABORTED) {
            return runAppImage(pathToAppImage, argc, argv);
        } else {
            return runAppImage(pathToIntegratedAppImage, argc, argv);
        }
    };

    // after checking whether the AppImage can/must be run without integrating it, we now check whether it actually
    // has been integrated already
    if (hasAlreadyBeenIntegrated(pathToAppImage)) {
        auto updateAndRunAppImage = [&pathToAppImage, argc, &argv]() {
            if (!updateDesktopFile(pathToAppImage))
                return 1;

            return runAppImage(pathToAppImage, argc, argv);
        };

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

    QMessageBox messageBox(
        QMessageBox::Question,
        QObject::tr("Desktop Integration"),
        QString::fromStdString(messageStrm.str())
    );

    auto* okButton = messageBox.addButton(QObject::tr("Integrate and run"), QMessageBox::AcceptRole);
    auto* runOnceButton = messageBox.addButton(QObject::tr("Run once"), QMessageBox::ApplyRole);

    // *whyever* Qt somehow needs a button with "RejectRole" or the X button won't close the current window...
    auto* cancelButton = messageBox.addButton(QObject::tr("Cancel"), QMessageBox::RejectRole);
    // ... but it is fine to hide that button after creating it, so it's not displayed
    cancelButton->hide();

    messageBox.setDefaultButton(QMessageBox::Ok);

    messageBox.exec();

    const auto* clickedButton = messageBox.clickedButton();

    if (clickedButton == okButton) {
        return integrateAndRunAppImage();
    } else if (clickedButton == runOnceButton) {
        return runAppImage(pathToAppImage, argc, argv);
    } else if (clickedButton == cancelButton) {
        return 0;
    }

    // _should_ be unreachable
    return 1;
}

