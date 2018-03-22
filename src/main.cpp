// system includes
#include <iostream>
#include <sstream>
extern "C" {
    #include <sys/stat.h>
    #include <unistd.h>
}

// library includes
#include <QApplication>
#include <QCommandLineParser>
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
extern "C" {
    #include <appimage/appimage.h>
}

// Runs an AppImage. Returns suitable exit code for main application.
int runAppImage(const QString& pathToAppImage) {
    // first of all, chmod +x the AppImage file, otherwise execv() will complain

    std::vector<char> cPath(static_cast<unsigned long>(pathToAppImage.size()) + 1, '\0');
    strcpy(cPath.data(), pathToAppImage.toStdString().c_str());

    struct stat appImageStat{};

    if (stat(cPath.data(), &appImageStat) != 0) {
        std::cerr << "Failed to call stat() on " << pathToAppImage.toStdString() << std::endl;
        return 1;
    }

    chmod(cPath.data(), appImageStat.st_mode | 0111);

    // fork and execv()
    int pid = fork();

    if (pid <= -1) {
        std::cout << "fork() error";
        return 1;
    } else if (pid == 0) {
        // TODO: find a way to use e.g. /lib64/ld-linux-x86-64.so.2 to bypass binfmt_misc when trying to call AppImage
        // alternatively, the AppImage would have to be mounted by this application, and AppRun would need to be called
        // however, this requires some process management (e.g., killing all processes inside the AppImage and also
        // the FUSE "mount" process, when this application is killed...)

        auto* args = new char*[2];
        args[0] = cPath.data();
        args[1] = nullptr;

        execv(cPath.data(), args);

        const auto& error = errno;
        std::cout << "execv() failed: " << strerror(error) << std::endl;

        delete[] args;
    } else {
        return 0;
    }
}

bool integrateAppImage(const QString& pathToAppImage) {
    auto rv = appimage_register_in_system(pathToAppImage.toStdString().c_str(), false);

    return rv == 0;
}

int main(int argc, char** argv) {
    QCommandLineParser parser;
    parser.setApplicationDescription("Desktop integration helper for AppImages, for use by Linux distributions");

    QApplication app(argc, argv);
    app.setApplicationDisplayName("AppImageLauncher");

    std::ostringstream version;
    version << "version " << APPIMAGELAUNCHER_VERSION << " "
            << "(git commit " << APPIMAGELAUNCHER_GIT_COMMIT << "), built on "
            << APPIMAGELAUNCHER_BUILD_DATE;
    app.setApplicationVersion(QString::fromStdString(version.str()));

    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);

    parser.addPositionalArgument("path", "Path to AppImage", "<path>");

    if (parser.positionalArguments().empty()) {
        parser.showHelp(1);
    }

    const auto pathToAppImage = parser.positionalArguments().first();

    if (!QFile(pathToAppImage).exists()) {
        std::cout << "Error: no such file or directory: " << pathToAppImage.toStdString() << std::endl;
        return 1;
    }

    if (appimage_is_registered_in_system(pathToAppImage.toStdString().c_str()))
        return runAppImage(pathToAppImage);

    std::ostringstream explanationStrm;
    explanationStrm << "Integrating it will move the AppImage into a predefined location, "
                    << "and include it in your application launcher." << "\n\n"
                    << "To remove or update the AppImage, please use the context menu of the application icon in "
                    << "your task bar.";

    auto explanation = explanationStrm.str();

    std::ostringstream messageStrm;
    messageStrm << pathToAppImage.toStdString() << " "
                << QObject::tr("has not been integrated into your system.").toStdString() << "\n\n"
                << QObject::tr(explanation.c_str()).toStdString();

    QMessageBox messageBox(
        QMessageBox::Question,
        "AppImage Desktop Integrator",
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
        if (!integrateAppImage(pathToAppImage))
            return 1;
        return runAppImage(pathToAppImage);
    } else if (clickedButton == runOnceButton) {
        return runAppImage(pathToAppImage);
    } else if (clickedButton == cancelButton) {
        return 0;
    }

    // _should_ be unreachable
    return 1;
}

