// system includes
#include <iostream>
#include <sstream>

// library includes
#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QPushButton>
#include <QMessageBox>
extern "C" {
    #include <appimage/appimage.h>
}

bool unregisterAppImage(const QString& pathToAppImage) {
    auto rv = appimage_unregister_in_system(pathToAppImage.toStdString().c_str(), false);

    if (rv != 0)
        return false;

    return true;
}

int main(int argc, char** argv) {
    QCommandLineParser parser;
    parser.setApplicationDescription("Removes AppImages after desktop integration, for use by Linux distributions");

    QApplication app(argc, argv);
    app.setApplicationDisplayName("AppImageLauncher remove");

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
        QMessageBox::critical(nullptr, "Error", QString::fromStdString("Error: no such file or directory: " + pathToAppImage.toStdString()));
        return 1;
    }

    const auto type = appimage_get_type(pathToAppImage.toStdString().c_str(), false);

    if (type <= 0 || type > 2) {
        QMessageBox::critical(nullptr, "AppImage remove helper error", "Not an AppImage: " + pathToAppImage);
        return 1;
    }

    auto clickedButton = QMessageBox::question(
        nullptr,
        "Please confirm",
        "Are you sure you want to remove this AppImage?\n\n" + pathToAppImage,
        QMessageBox::No | QMessageBox::Yes,
        QMessageBox::No
    );

    switch (clickedButton) {
        case QMessageBox::Yes: {
            // first, unregister AppImage
            auto path = pathToAppImage.toStdString();

            if (appimage_unregister_in_system(path.c_str(), false) != 0) {
                QMessageBox::critical(nullptr, "Error", QString::fromStdString("Failed to unregister AppImage: " + path));
                return 1;
            }

            // now, remove AppImage file
            if (std::remove(path.c_str()) != 0) {
                auto error = errno;
                QMessageBox::critical(nullptr, "Error", QString::fromStdString("Failed to remove AppImage: " + std::string(strerror(error))));
            }
        }
        default: {
            // exit without any actions
            return 0;
        }
    }


    // _should_ be unreachable
    return 1;
}

