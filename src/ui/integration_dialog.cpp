// system includes
#include <sstream>
#include <utility>

// library includes
#include <QStyle>

// local headers
#include "integration_dialog.h"
#include "ui_integration_dialog.h"

IntegrationDialog::IntegrationDialog(QString pathToAppImage, QString integratedAppImagesDestinationPath,
                                     bool integrateInPlace, QWidget* parent) :
    QDialog(parent), ui(new Ui::IntegrationDialog),
    pathToAppImage(std::move(pathToAppImage)),
    integratedAppImagesDestinationPath(std::move(integratedAppImagesDestinationPath)),
    integrateInPlace(integrateInPlace) {
    ui->setupUi(this);

    setIcon();
    setMessage();

    QObject::connect(ui->pushButtonIntegrateAndRun, &QPushButton::released, this,
                     &IntegrationDialog::onPushButtonIntegrateAndRunReleased);
    QObject::connect(ui->pushButtonRunOnce, &QPushButton::released, this,
                     &IntegrationDialog::onPushButtonRunOnceReleased);

    // make translation fit by adjusting the minimum size of the message label to the size calculated by Qt
    ui->message->setMinimumSize(ui->message->sizeHint());
}

void IntegrationDialog::setMessage() {
    // the message designed in the .ui file describes integrating and moving the AppImage into the integration
    // destination directory; when the AppImage is going to be integrated in place, use an alternate message
    // which does not mention the move
    if (integrateInPlace) {
        ui->message->setText(
            tr("%1 has not been integrated into your system.\n"
               "\n"
               "Integrating it will include the AppImage in your application launcher, and leave the file in its "
               "current location.\n"
               "\n"
               "To remove or update the AppImage, please use the context menu of the application icon in your "
               "task bar or launcher.").arg(pathToAppImage)
        );
        return;
    }

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
