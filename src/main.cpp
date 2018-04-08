// system includes
#include <fstream>
#include <iostream>
#include <sstream>
extern "C" {
    #include <sys/stat.h>
    #include <libgen.h>
    #include <unistd.h>
    #include <glib.h>
    #include <xdg-basedir.h>
}

// library includes
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QMessageBox>
#include <QRegularExpression>
#include <QString>
extern "C" {
    #include <appimage/appimage.h>
}

bool makeExecutable(const std::string& path) {
    struct stat fileStat{};

    if (stat(path.c_str(), &fileStat) != 0) {
        std::cerr << "Failed to call stat() on " << path << std::endl;
        return false;
    }

    // no action required when file is executable already
    // this could happen in scenarios when an AppImage is in a read-only location
    if ((fileStat.st_uid == getuid() && fileStat.st_mode & 0100) ||
        (fileStat.st_gid == getgid() && fileStat.st_mode & 0010) ||
        (fileStat.st_mode & 0001)) {
        return true;
    }

    return chmod(path.c_str(), fileStat.st_mode | 0111) == 0;
}

// Runs an AppImage. Returns suitable exit code for main application.
int runAppImage(const QString& pathToAppImage, int argc, char** argv) {
    // needs to be converted to std::string to be able to use c_str()
    // when using QString and then .toStdString().c_str(), the std::string instance will be an rvalue, and the
    // pointer returned by c_str() will be invalid
    auto x = pathToAppImage.toStdString();
    auto fullPathToAppImage = QFileInfo(pathToAppImage).absoluteFilePath().toStdString();

    if (appimage_get_type(fullPathToAppImage.c_str(), false) < 2) {
        QMessageBox::critical(
            nullptr,
            "AppImageLauncher error",
            "AppImageLauncher does not support type 1 AppImages at the moment."
        );
        return 1;
    }

    // first of all, chmod +x the AppImage file, otherwise execv() will complain
    if (!makeExecutable(fullPathToAppImage)) {
        QMessageBox::critical(nullptr, "Error", QString::fromStdString("Could not make AppImage executable: " + fullPathToAppImage));
        return 1;
    }

    // build path to AppImage runtime
    // as it might error, check before fork()ing to be able to display an error message beforehand
    auto exeDir = QFileInfo(QFile("/proc/self/exe").symLinkTarget()).absoluteDir().absolutePath();

    // use external runtime _without_ magic bytes to run the AppImage
    // the original idea was to use /lib64/ld-linux-x86_64.so to run AppImages, but it did complain about some
    // violated ELF data, and refused to run the AppImage
    // alternatively, the AppImage would have to be mounted by this application, and AppRun would need to be called
    // however, this requires some process management (e.g., killing all processes inside the AppImage and also
    // the FUSE "mount" process, when this application is killed...)
    setenv("TARGET_APPIMAGE", fullPathToAppImage.c_str(), true);

    // first attempt: find runtime in expected installation directory
    auto pathToRuntime = exeDir.toStdString() + "/../lib/appimagelauncher/runtime";

    // next method: find runtime in expected build location
    if (!QFile(QString::fromStdString(pathToRuntime)).exists()) {
        pathToRuntime = exeDir.toStdString() + "/../lib/AppImageKit/src/runtime";
    }

    // if it can't be found in either location, display error and exit
    if (!QFile(QString::fromStdString(pathToRuntime)).exists()) {
        QMessageBox::critical(nullptr, "Error", QString::fromStdString("runtime not found: no such file or directory: " + pathToRuntime));
        return 1;
    }

    // need a char pointer instead of a const one, therefore can't use .c_str()
    std::vector<char> fullPathToAppImageBuf(pathToRuntime.size() + 1, '\0');
    strcpy(fullPathToAppImageBuf.data(), pathToRuntime.c_str());

    std::vector<char*> args;
    args.push_back(fullPathToAppImageBuf.data());

    // copy arguments
    for (int i = 1; i < argc; i++) {
        args.push_back(argv[i]);
    }

    // args need to be null terminated
    args.push_back(nullptr);

    execv(pathToRuntime.c_str(), args.data());

    const auto& error = errno;
    std::cout << "execv() failed: " << strerror(error) << std::endl;
}

QMap<QString, QString> findCollisions(const QString& currentNameEntry) {
    QMap<QString, QString> collisions;

    // default locations of desktop files on systems
    const auto directories = {QString("/usr/share/applications/"), QString(xdg_data_home()) + "/applications/"};

    for (const auto& directory : directories) {
        QDirIterator iterator(directory, QDirIterator::FollowSymlinks);

        while (iterator.hasNext()) {
            const auto& filename = iterator.next();

            if (!QFileInfo(filename).isFile() || !filename.endsWith(".desktop"))
                continue;

            GKeyFile* desktopFile = g_key_file_new();
            GError* error = nullptr;

            auto cleanup = [&desktopFile, &error]() {
                if (desktopFile != nullptr) {
                    g_key_file_free(desktopFile);
                    desktopFile = nullptr;
                }

                if (error != nullptr) {
                    g_error_free(error);
                    error = nullptr;
                }
            };

            // if the key file parser can't load the file, it's most likely not a valid desktop file, so we just skip this file
            if (!g_key_file_load_from_file(desktopFile, filename.toStdString().c_str(), G_KEY_FILE_KEEP_TRANSLATIONS, &error)) {
                cleanup();
                continue;
            }

            auto* nameEntry = g_key_file_get_string(desktopFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, &error);

            // invalid desktop file, needs to be skipped
            if (nameEntry == nullptr) {
                cleanup();
                continue;
            }

            if (QString(nameEntry).trimmed().startsWith(currentNameEntry.trimmed())) {
                collisions[filename] = QString(nameEntry);
            }

            cleanup();
        }
    }

    return collisions;
}

bool integrateAppImage(const QString& pathToAppImage, const QString& pathToIntegratedAppImage) {
    // need std::strings to get working pointers with .c_str()
    const auto oldPath = pathToAppImage.toStdString();
    const auto newPath = pathToIntegratedAppImage.toStdString();

    // create target directory
    QDir().mkdir(QFileInfo(QFile(pathToIntegratedAppImage)).dir().absolutePath());

    // need to check whether file exists
    // if it does, the existing AppImage needs to be removed before rename can be called
    if (QFile(pathToIntegratedAppImage).exists()) {
        QFile(pathToIntegratedAppImage).remove();
    }
    if (!QFile(pathToAppImage).rename(pathToIntegratedAppImage)) {
        QMessageBox::critical(nullptr, "Error", "Failed to move AppImage to target location");
        return false;
    }

    if (appimage_register_in_system(newPath.c_str(), false) != 0)
        return false;

    const auto* desktopFilePath = appimage_registered_desktop_file_path(newPath.c_str(), nullptr, false);

    // sanity check -- if the file doesn't exist, the function returns NULL
    if (desktopFilePath == nullptr) {
        QMessageBox::critical(nullptr, "Error", "Failed to find integrated desktop file");
        return false;
    }

    // check that file exists
    if (!QFile(desktopFilePath).exists())
        return false;

    /* write AppImageLauncher specific entries to desktop file
     *
     * unfortunately, QSettings doesn't work as a desktop file reader/writer, and libqtxdg isn't really meant to be
     * used by projects via add_subdirectory/ExternalProject
     * a system dependency is not an option for this project, and we link to glib already anyway, so let's just use
     * glib, which is known to work
     */
    auto desktopFile = g_key_file_new();

    GError *error = nullptr;
    const auto flags = GKeyFileFlags(G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS);

    // for people who don't want to create memory leaks when using C APIs, C++11's lambdas provide a great option to
    // clean up those data
    auto cleanup = [&error, &desktopFile]() {
        if (desktopFile != nullptr)
            g_key_file_free(desktopFile);

        if (error != nullptr)
            g_error_free(error);
    };

    auto handleError = [&error, &desktopFile, &cleanup]() {
        std::ostringstream ss;
        ss << "Failed to load desktop file: " << std::endl << error->message;
        QMessageBox::critical(nullptr, "Error", QString::fromStdString(ss.str()));

        cleanup();
    };

    if (!g_key_file_load_from_file(desktopFile, desktopFilePath, flags, &error)) {
        handleError();
        return false;
    }

    const auto* nameEntry = g_key_file_get_string(desktopFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, &error);

    if (nameEntry == nullptr) {
        QMessageBox::warning(nullptr, "Warning", "AppImage has invalid desktop file");
    }

    // TODO: support multilingual collisions
    auto collisions = findCollisions(nameEntry);

    // make sure to remove own entry
    collisions.remove(QString(desktopFilePath));

    if (!collisions.empty()) {
        // collisions are resolved like in the filesystem: a monotonically increasing number in brackets is appended to
        // the Name
        // in order to keep the number monotonically increasing, we look for the highest number in brackets in the
        // existing entries, add 1 to it, and append it in brackets to the current desktop file's Name entry

        unsigned int currentNumber = 1;

        QRegularExpression regex("^.*([0-9]+)$");

        for (const auto& fileName : collisions) {
            const auto& currentNameEntry = collisions[fileName];

            auto match = regex.match(currentNameEntry);

            if (match.hasMatch()) {
                const unsigned int num = match.captured(0).toUInt();
                if (num >= currentNumber)
                    currentNumber = num + 1;
            }
        }

        auto newName = QString(nameEntry) + " (" + QString::number(currentNumber) + ")";
        g_key_file_set_string(desktopFile, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_NAME, newName.toStdString().c_str());
    }

    auto convertToCharPointerList = [](const std::vector<std::string>& stringList) {
        std::vector<const char*> pointerList;

        // reserve space to increase efficiency
        pointerList.reserve(stringList.size());

        // convert string list to list of const char pointers
        for (const auto& action : stringList) {
            pointerList.push_back(action.c_str());
        }

        return pointerList;
    };

    std::vector<std::string> desktopActions = {"Remove"};

    g_key_file_set_string_list(
        desktopFile,
        G_KEY_FILE_DESKTOP_GROUP,
        G_KEY_FILE_DESKTOP_KEY_ACTIONS,
        convertToCharPointerList(desktopActions).data(),
        desktopActions.size()
    );

    // add Remove action
    const auto removeSectionName = "Desktop Action Remove";

    g_key_file_set_string(desktopFile, removeSectionName, "Name", "Remove from system");

    // TODO: properly escape path -- single quotes are not failsafe
    // we should probably write a library supporting the desktop file standard's escaping, for use in libappimage
    std::ostringstream execPath;
    execPath << CMAKE_INSTALL_PREFIX << "/lib/appimagelauncher/remove " << newPath;
    g_key_file_set_string(desktopFile, removeSectionName, "Exec", execPath.str().c_str());

    if (!g_key_file_save_to_file(desktopFile, desktopFilePath, &error)) {
        handleError();
        return false;
    }

    cleanup();
    return true;
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationDisplayName("AppImageLauncher");

    std::ostringstream version;
    version << "version " << APPIMAGELAUNCHER_VERSION << " "
            << "(git commit " << APPIMAGELAUNCHER_GIT_COMMIT << "), built on "
            << APPIMAGELAUNCHER_BUILD_DATE;
    app.setApplicationVersion(QString::fromStdString(version.str()));

    std::ostringstream usage;
    usage << "Usage: " << argv[0] << " [options] <path>" << std::endl
          << "Desktop integration helper for AppImages, for use by Linux distributions." << std::endl
          << std::endl
          << "Options:" << std::endl
          << "  --appimagelauncher-help     Display this help and exit" << std::endl
          << "  --appimagelauncher-version  Display version and exit" << std::endl
          << std::endl
          << "Arguments:"
          << "  path                        Path to AppImage (mandatory)" << std::endl;

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

    // search for appimagelauncher arguments in args list
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
            } else {
                std::cerr << "Unknown AppImageLauncher option: " << arg.toStdString() << std::endl;
                return 1;
            }
        }
    }

    auto pathToAppImage = QString(argv[1]);

    if (!QFile(pathToAppImage).exists()) {
        std::cout << "Error: no such file or directory: " << pathToAppImage.toStdString() << std::endl;
        return 1;
    }

    const auto type = appimage_get_type(pathToAppImage.toStdString().c_str(), false);

    if (type <= 0) {
        QMessageBox::critical(nullptr, "AppImageLauncher error", "Not an AppImage: " + pathToAppImage);
        return 1;
    }

    if (type == 1) {
        QMessageBox::critical(
            nullptr,
            "AppImageLauncher error",
            "AppImageLauncher does not support type 1 AppImages at the moment."
        );
        return 1;
    }

    auto integratedAppImagesDestination = QString(getenv("HOME")) + "/.bin/";

    auto pathToIntegratedAppImage = integratedAppImagesDestination + basename(const_cast<char*>(pathToAppImage.toStdString().c_str()));

    // AppImages in AppImages are not supposed to be integrated
    if (pathToAppImage.startsWith("/tmp/.mount_"))
        return runAppImage(pathToAppImage, argc, argv);

    // check whether AppImage has been integrated
    if (appimage_is_registered_in_system(pathToAppImage.toStdString().c_str()))
        return runAppImage(pathToAppImage, argc, argv);

    // ignore terminal apps (fixes #2)
    if (appimage_is_terminal_app(pathToAppImage.toStdString().c_str()))
        return runAppImage(pathToAppImage, argc, argv);

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

    std::ostringstream explanationStrm;
    explanationStrm << "Integrating it will move the AppImage into a predefined location, "
                    << "and include it in your application launcher." << std::endl
                    << std::endl
                    << "To remove or update the AppImage, please use the context menu of the application icon in "
                    << "your task bar or launcher." << std::endl
                    << std::endl
                    << "The directory the integrated AppImages are stored in is currently set to:" << std::endl
                    << integratedAppImagesDestination.toStdString() << std::endl;

    auto explanation = explanationStrm.str();

    std::ostringstream messageStrm;
    messageStrm << pathToAppImage.toStdString() << " "
                << QObject::tr("has not been integrated into your system.").toStdString() << "\n\n"
                << QObject::tr(explanation.c_str()).toStdString();

    QMessageBox messageBox(
        QMessageBox::Question,
        "Desktop Integration",
        QString::fromStdString(messageStrm.str())
    );

    auto* okButton = messageBox.addButton("Integrate and run", QMessageBox::AcceptRole);
    auto* runOnceButton = messageBox.addButton("Run once", QMessageBox::ApplyRole);

    // *whyever* Qt somehow needs a button with "RejectRole" or the X button won't close the current window...
    auto* cancelButton = messageBox.addButton("Cancel", QMessageBox::RejectRole);
    // ... but it is fine to hide that button after creating it, so it's not displayed
    cancelButton->hide();

    messageBox.setDefaultButton(QMessageBox::Ok);

    messageBox.exec();

    const auto* clickedButton = messageBox.clickedButton();

    if (clickedButton == okButton) {
        if (!integrateAppImage(pathToAppImage, pathToIntegratedAppImage))
            return 1;
        return runAppImage(pathToIntegratedAppImage, argc, argv);
    } else if (clickedButton == runOnceButton) {
        return runAppImage(pathToAppImage, argc, argv);
    } else if (clickedButton == cancelButton) {
        return 0;
    }

    // _should_ be unreachable
    return 1;
}

