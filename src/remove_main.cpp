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
#include "trashbin.h"

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
            if (!unregisterAppImage(pathToAppImage)) {
                QMessageBox::critical(
                    nullptr,
                    QObject::tr("Error"),
                    QObject::tr("Failed to unregister AppImage: %1").arg(pathToAppImage)
                );
                return 1;
            }

            TrashBin bin;

            // now, move AppImage into trash bin
            if (!bin.disposeAppImage(pathToAppImage)) {
                QMessageBox::critical(
                    nullptr,
                    QObject::tr("Error"),
                    QObject::tr("Failed to move AppImage into trash bin directory")
                );
                return 1;
            }

            // run clean up cycle for trash bin
            // if the current AppImage is ready to be deleted, this call will immediately remove it from the system
            // otherwise, it'll be cleaned up at some subsequent run of AppImageLauncher or the removal tool
            if (!bin.cleanUp()) {
                QMessageBox::critical(
                    nullptr,
                    QObject::tr("Error"),
                    QObject::tr("Failed to clean up AppImage trash bin: %1").arg(bin.path())
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

