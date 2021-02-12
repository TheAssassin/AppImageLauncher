#include <QStyle>
#include "integration_dialog.h"
#include "ui_integration_dialog.h"

IntegrationDialog::IntegrationDialog(QWidget* parent) :
    QDialog(parent), ui(new Ui::IntegrationDialog) {
    ui->setupUi(this);

    QIcon icon = style()->standardIcon(QStyle::SP_MessageBoxQuestion);
    QPixmap pixmap = icon.pixmap(QSize(64, 64));

    ui->icon->setPixmap(pixmap);
}

IntegrationDialog::~IntegrationDialog() {
    delete ui;
}
