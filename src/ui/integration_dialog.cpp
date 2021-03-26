// system includes
#include <sstream>
#include <utility>

// library includes
#include <QStyle>

// local headers
#include "integration_dialog.h"
#include "ui_integration_dialog.h"

IntegrationDialog::IntegrationDialog(QString pathToAppImage, QString integratedAppImagesDestinationPath,
                                     QWidget* parent) :
    QDialog(parent), ui(new Ui::IntegrationDialog),
    pathToAppImage(std::move(pathToAppImage)),
    integratedAppImagesDestinationPath(std::move(integratedAppImagesDestinationPath)) {
    ui->setupUi(this);

    setIcon();
    setMessage();

    this->setResult(Options::DO_NOTHING);
    QObject::connect(ui->pushButtonIntegrateAndRun, &QPushButton::released, this,
                     &IntegrationDialog::onPushButtonIntegrateAndRunReleased);
    QObject::connect(ui->pushButtonRunOnce, &QPushButton::released, this,
                     &IntegrationDialog::onPushButtonRunOnceReleased);
}

void IntegrationDialog::setMessage() {
    QString message = ui->message->text();
    message = message.arg(pathToAppImage, integratedAppImagesDestinationPath);
    ui->message->setText(message);
}

void IntegrationDialog::setIcon() {
    QIcon icon = QIcon(":/AppImageLauncher.svg");
    QPixmap pixmap = icon.pixmap(QSize(64, 64));
    ui->icon->setPixmap(pixmap);
}

IntegrationDialog::~IntegrationDialog() {
    delete ui;
}

void IntegrationDialog::onPushButtonIntegrateAndRunReleased() {
    this->done(Options::INTEGRATE_AND_RUN);
}

void IntegrationDialog::onPushButtonRunOnceReleased() {
    this->done(Options::RUN_ONCE);
}
