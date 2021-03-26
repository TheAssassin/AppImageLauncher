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
    std::ostringstream explanationStrm;
    explanationStrm << QObject::tr("Integrating it will move the AppImage into a predefined location, "
                                   "and include it in your application launcher.").toStdString() << std::endl
                    << std::endl
                    << QObject::tr("To remove or update the AppImage, please use the context menu of the "
                                   "application icon in your task bar or launcher.").toStdString() << std::endl
                    << std::endl
                    << QObject::tr("The directory the integrated AppImages are stored in is currently "
                                   "set to:").toStdString() << std::endl
                    << integratedAppImagesDestinationPath.toStdString() << std::endl;

    auto explanation = explanationStrm.str();

    std::ostringstream messageStrm;
    messageStrm << QObject::tr("%1 has not been integrated into your system.").arg(pathToAppImage).toStdString()
                << "\n\n"
                << QObject::tr(explanation.c_str()).toStdString();
    ui->message->setText(QString::fromStdString(messageStrm.str()));
}

void IntegrationDialog::setIcon() {
    QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxQuestion);
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
