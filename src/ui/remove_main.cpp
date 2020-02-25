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
#include "ui_remove.h"

int main(int argc, char** argv) {
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Helper to delete integrated AppImages easily, e.g., from the application launcher's context menu"));
    QApplication app(argc, argv);
    QApplication::setApplicationDisplayName("AppImageLauncher");
    QApplication::setWindowIcon(QIcon(":/AppImageLauncher.svg"));

    std::ostringstream version;
    version << "version " << APPIMAGELAUNCHER_VERSION << " "
            << "(git commit " << APPIMAGELAUNCHER_GIT_COMMIT << "), built on "
            << APPIMAGELAUNCHER_BUILD_DATE;
    QApplication::setApplicationVersion(QString::fromStdString(version.str()));

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

    checkAuthorizationAndShowDialogIfNecessary(pathToAppImage, "Delete anyway?");

    const auto type = appimage_get_type(pathToAppImage.toStdString().c_str(), false);

    if (type <= 0 || type > 2) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("AppImage delete helper error"),
            QObject::tr("Not an AppImage:\n\n%1").arg(pathToAppImage)
        );
        return 1;
    }

    // this tool should not do anything if the file isn't integrated
    // the file is only supposed to work on integrated AppImages and _nothing else_
//    if (!hasAlreadyBeenIntegrated(pathToAppImage)) {
//        QMessageBox::critical(
//                nullptr,
//                QObject::tr("AppImage delete helper error"),
//                QObject::tr("Refusing to work on non-integrated AppImage:\n\n%1").arg(pathToAppImage)
//        );
//        return 1;
//    }

    QDialog dialog;
    Ui::RemoveDialog ui;

    ui.setupUi(&dialog);
    ui.pathLabel->setText(pathToAppImage);

    // must be done *after* loading the UI into the dialog
    setUpFallbackIconPaths(&dialog);

    auto rv = dialog.exec();

    // check if user has canceled the dialog
    // a confirmation would result in an exit code of 1
    if (rv != 1) {
        return 0;
    }

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

    return 0;
}

