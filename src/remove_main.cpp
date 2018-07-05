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
#include <QLibraryInfo>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QTranslator>
extern "C" {
    #include <appimage/appimage.h>
}

// local includes
#include "shared.h"
#include "translationmanager.h"

bool unregisterAppImage(const QString& pathToAppImage) {
    auto rv = appimage_unregister_in_system(pathToAppImage.toStdString().c_str(), false);

    if (rv != 0)
        return false;

    return true;
}

int main(int argc, char** argv) {
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Removes AppImages after desktop integration, for use by Linux distributions"));
    QApplication app(argc, argv);
    app.setApplicationDisplayName(QObject::tr("AppImageLauncher remove", "remove helper app name"));

    std::ostringstream version;
    version << "version " << APPIMAGELAUNCHER_VERSION << " "
            << "(git commit " << APPIMAGELAUNCHER_GIT_COMMIT << "), built on "
            << APPIMAGELAUNCHER_BUILD_DATE;
    app.setApplicationVersion(QString::fromStdString(version.str()));

    // install translations
    TranslationManager translationManager(app);

    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);

    parser.addPositionalArgument("path", QObject::tr("Path to AppImage"), QObject::tr("<path>"));

    if (parser.positionalArguments().empty()) {
        parser.showHelp(1);
    }

    const auto pathToAppImage = parser.positionalArguments().first();

    if (!QFile(pathToAppImage).exists()) {
        QMessageBox::critical(nullptr, "Error", QObject::tr("Error: no such file or directory: %1").arg(pathToAppImage));
        return 1;
    }

    const auto type = appimage_get_type(pathToAppImage.toStdString().c_str(), false);

    if (type <= 0 || type > 2) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("AppImage remove helper error"), QObject::tr("Not an AppImage: %1").arg(pathToAppImage)
        );
        return 1;
    }

    auto clickedButton = QMessageBox::question(
        nullptr,
        QObject::tr("Please confirm"),
        QObject::tr("Are you sure you want to remove this AppImage?") + "\n\n" + pathToAppImage,
        QMessageBox::No | QMessageBox::Yes,
        QMessageBox::No
    );

    switch (clickedButton) {
        case QMessageBox::Yes: {
            // first, unregister AppImage
            auto path = pathToAppImage.toStdString();

            if (!unregisterAppImage(pathToAppImage)) {
                QMessageBox::critical(
                    nullptr,
                    QObject::tr("Error"),
                    QObject::tr("Failed to unregister AppImage: %1").arg(QString::fromStdString(path))
                );
                return 1;
            }

            // now, remove AppImage file
            if (std::remove(path.c_str()) != 0) {
                auto error = errno;
                QMessageBox::critical(
                    nullptr,
                    QObject::tr("Error"),
                    QObject::tr("Failed to remove AppImage: %1").arg(error)
                );
                return 1;
            }
            
            // update desktop database and icon caches
            if (!updateDesktopDatabaseAndIconCaches())
                return 1;
        }
        default: {
            // exit without any actions
            return 0;
        }
    }


    // _should_ be unreachable
    return 1;
}

