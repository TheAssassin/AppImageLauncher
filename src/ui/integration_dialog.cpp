// system includes
#include <sstream>
#include <utility>

// library includes
#include <QStyle>

#include <XdgUtils/DesktopEntry/DesktopEntry.h>
#include <appimage/utils/ResourcesExtractor.h>


// local headers
#include "integration_dialog.h"
#include "ui_integration_dialog.h"

IntegrationDialog::IntegrationDialog(QString pathToAppImage, QString integratedAppImagesDestinationPath,
                                     QWidget* parent) :
    QDialog(parent), ui(new Ui::IntegrationDialog),
    pathToAppImage(std::move(pathToAppImage)),
    integratedAppImagesDestinationPath(std::move(integratedAppImagesDestinationPath)) {
    ui->setupUi(this);

    loadAppImageInfo();

    QObject::connect(ui->pushButtonIntegrateAndRun, &QPushButton::released,
                     this, &IntegrationDialog::onPushButtonIntegrateAndRunReleased);
    QObject::connect(ui->pushButtonRunOnce, &QPushButton::released,
                     this, &IntegrationDialog::onPushButtonRunOnceReleased);
}

IntegrationDialog::~IntegrationDialog() {
    delete ui;
}

void IntegrationDialog::onPushButtonIntegrateAndRunReleased() {
    this->resultAction = ResultingAction::IntegrateAndRun;
    this->accept();
}

void IntegrationDialog::onPushButtonRunOnceReleased() {
    this->resultAction = ResultingAction::RunOnce;
    this->accept();
}

IntegrationDialog::ResultingAction IntegrationDialog::getResultAction() const {
    return resultAction;
}

void IntegrationDialog::loadAppImageInfo() {
    try {
        appimage::core::AppImage appImage(pathToAppImage.toStdString());
        appimage::utils::ResourcesExtractor extractor(appImage);

        auto desktopEntryPath = extractor.getDesktopEntryPath();
        auto desktopEntryData = extractor.extractText(desktopEntryPath);

        XdgUtils::DesktopEntry::DesktopEntry desktopEntry(desktopEntryData);
        auto appName = QString::fromStdString(desktopEntry.get("Desktop Entry/Name", ""));
        auto appDescription = QString::fromStdString(desktopEntry.get("Desktop Entry/Comment", ""));

        ui->labelName->setText(appName);
        ui->labelDescription->setText(appDescription);

        // to keep the text aligned in the center with the icon
        if (appDescription.isEmpty()) {
            ui->labelDescription->setVisible(false);
            ui->labelName->setAlignment(Qt::AlignVCenter);
        }

        // Read icon data from ".DirIcon"
        std::vector<char> iconData = extractor.extract(".DirIcon");

        // Load into a pixmap
        QPixmap pixmap;
        pixmap.loadFromData(reinterpret_cast<const uchar*>(iconData.data()), iconData.size());

        // Fallback to the AppImageLauncher icon in case of error
        if (pixmap.isNull())
            pixmap = QPixmap(":/AppImageLauncher.svg");

        // scale icon to 64x64
        pixmap = pixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->icon->setPixmap(pixmap);

        // Replace Integrated AppImages Destination Path in the message label
        QString message = ui->message->text();
        message = message.arg(integratedAppImagesDestinationPath);
        ui->message->setText(message);
    } catch (appimage::core::AppImageError& error) {
        // TODO: Properly handle errors
    }
}
