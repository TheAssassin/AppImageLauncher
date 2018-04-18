// system includes
#include <iostream>
#include <sstream>

// library includes
#include <QApplication>
#include <QCheckBox>
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
#include <appimage/update/qt-ui.h>

// local includes
#include "shared.h"


int main(int argc, char** argv) {
    QCommandLineParser parser;
    parser.setApplicationDescription("Updates AppImages after desktop integration, for use by Linux distributions");

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

    auto criticalUpdaterError = [](const QString& message) {
        QMessageBox::critical(nullptr, "Error", message);
    };

    if (!QFile(pathToAppImage).exists()) {
        criticalUpdaterError(QString::fromStdString("Error: no such file or directory: " + pathToAppImage.toStdString()));
        return 1;
    }

    const auto type = appimage_get_type(pathToAppImage.toStdString().c_str(), false);

    if (type <= 0 || type > 2) {
        criticalUpdaterError("Not an AppImage: " + pathToAppImage);
        return 1;
    }

    bool hasBeenRegisteredBefore;

    if (!(hasBeenRegisteredBefore = appimage_is_registered_in_system(pathToAppImage.toStdString().c_str()))) {
        QString message = "The AppImage hasn't been integrated before. This tool will, however, integrate the "
                          "updated AppImage."
                          "\n\n"
                          "Do you wish to continue?";

        switch (QMessageBox::warning(nullptr, "Warning", message, QMessageBox::Ok | QMessageBox::Cancel)) {
            case (QMessageBox::Ok):
                break;
            default:
                return 0;
        }
    }

    appimage::update::qt::QtUpdater updater(pathToAppImage);
    updater.enableRunUpdatedAppImageButton(false);

    if (!updater.checkForUpdates()) {
        QMessageBox::information(nullptr, "No updates found", "Could not find updates for AppImage " + pathToAppImage);
        return 0;
    }

    bool removeAfterUpdate = false;

    {
        const auto message = "An update has been found for the AppImage " + pathToAppImage + ".\n\n" +
                             "Do you want to perform the update?\n";

        QMessageBox messageBox(QMessageBox::Icon::Question, "Update found", message, QMessageBox::Ok | QMessageBox::Cancel);

        QCheckBox removeCheckBox("Remove old AppImage after successful update");
        removeCheckBox.setChecked(false);

        messageBox.setCheckBox(&removeCheckBox);

        switch (messageBox.exec()) {
            case QMessageBox::Ok:
                break;
            default:
                return 0;
        }

        removeAfterUpdate = removeCheckBox.isChecked();
    }

    // perform update
    updater.show();

    auto rv = app.exec();

    // if the update has failed, return immediately
    if (rv != 0)
        return rv;

    // get path to new file, un-integrate old file, remove it, and register updated AppImage
    QString pathToUpdatedAppImage;

    if (!updater.pathToNewFile(pathToUpdatedAppImage))
        return 1;

    // sanity check
    if (!QFile::exists(pathToUpdatedAppImage)) {
        criticalUpdaterError("File reported as updated does not exist: " + pathToUpdatedAppImage);
        return 1;
    }

    if (!appimage_shall_not_be_integrated(pathToAppImage.toStdString().c_str())) {
        const auto pathToIntegratedAppImage = buildPathToIntegratedAppImage(pathToAppImage);

        if (!integrateAppImage(pathToUpdatedAppImage, pathToIntegratedAppImage)) {
            criticalUpdaterError("Failed to register updated AppImage in system");
            return 1;
        }
    }

    if (removeAfterUpdate) {
        if (hasBeenRegisteredBefore && !appimage_unregister_in_system(pathToAppImage.toStdString().c_str(), false)) {
            criticalUpdaterError("Failed to unregister old AppImage in system");
            return 1;
        }

        if (!QFile::remove(pathToAppImage)) {
            criticalUpdaterError("Failed to remove old AppImage");
            return 1;
        }
    }

    // we're done!
    return 0;
}
