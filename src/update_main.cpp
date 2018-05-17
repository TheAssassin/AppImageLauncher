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
#include <QLibraryInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QTranslator>
extern "C" {
    #include <appimage/appimage.h>
}
#include <appimage/update/qt-ui.h>

// local includes
#include "shared.h"


int main(int argc, char** argv) {
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Updates AppImages after desktop integration, for use by Linux distributions"));

    QApplication app(argc, argv);
    app.setApplicationDisplayName(QObject::tr("AppImageLauncher remove", "remove helper app name"));

    std::ostringstream version;
    version << "version " << APPIMAGELAUNCHER_VERSION << " "
            << "(git commit " << APPIMAGELAUNCHER_GIT_COMMIT << "), built on "
            << APPIMAGELAUNCHER_BUILD_DATE;
    app.setApplicationVersion(QString::fromStdString(version.str()));

    // set up translations
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app.installTranslator(&qtTranslator);

    QTranslator myappTranslator;
    myappTranslator.load("appimagelauncher_" + QLocale::system().name());
    app.installTranslator(&myappTranslator);

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
        criticalUpdaterError(QString::fromStdString(QObject::tr("Error: no such file or directory: %1").arg(pathToAppImage).toStdString()));
        return 1;
    }

    const auto type = appimage_get_type(pathToAppImage.toStdString().c_str(), false);

    if (type <= 0 || type > 2) {
        criticalUpdaterError(QObject::tr("Not an AppImage: %1").arg(pathToAppImage));
        return 1;
    }

    bool hasBeenRegisteredBefore;

    if (!(hasBeenRegisteredBefore = appimage_is_registered_in_system(pathToAppImage.toStdString().c_str()))) {
        QString message = QObject::tr("The AppImage hasn't been integrated before. This tool will, however, integrate the "
                          "updated AppImage.") +
                          "\n\n" +
                          QObject::tr("Do you wish to continue?");

        switch (QMessageBox::warning(nullptr, QObject::tr("Warning"), message, QMessageBox::Ok | QMessageBox::Cancel)) {
            case (QMessageBox::Ok):
                break;
            default:
                return 0;
        }
    }

    appimage::update::qt::QtUpdater updater(pathToAppImage);
    updater.enableRunUpdatedAppImageButton(false);

    if (!updater.checkForUpdates()) {
        QMessageBox::information(
            nullptr,
            QObject::tr("No updates found"),
            QObject::tr("Could not find updates for AppImage") + " " + pathToAppImage);
        return 0;
    }

    bool removeAfterUpdate = false;

    {
        const auto message = QObject::tr("An update has been found for the AppImage %1").arg(pathToAppImage) +
                             "\n\n" +
                             QObject::tr("Do you want to perform the update?") + "\n";

        QMessageBox messageBox(QMessageBox::Icon::Question, "Update found", message, QMessageBox::Ok | QMessageBox::Cancel);

        QCheckBox removeCheckBox(QObject::tr("Remove old AppImage after successful update"));
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
        criticalUpdaterError(QObject::tr("File reported as updated does not exist: %1").arg(pathToUpdatedAppImage));
        return 1;
    }

    if (!appimage_shall_not_be_integrated(pathToAppImage.toStdString().c_str())) {
        const auto pathToIntegratedAppImage = buildPathToIntegratedAppImage(pathToAppImage);

        if (!integrateAppImage(pathToUpdatedAppImage, pathToIntegratedAppImage)) {
            criticalUpdaterError(QObject::tr("Failed to register updated AppImage in system"));
            return 1;
        }
    }

    if (removeAfterUpdate) {
        if (hasBeenRegisteredBefore && !appimage_unregister_in_system(pathToAppImage.toStdString().c_str(), false)) {
            criticalUpdaterError(QObject::tr("Failed to unregister old AppImage in system"));
            return 1;
        }

        if (!QFile::remove(pathToAppImage)) {
            criticalUpdaterError(QObject::tr("Failed to remove old AppImage"));
            return 1;
        }
    }

    // we're done!
    return 0;
}
