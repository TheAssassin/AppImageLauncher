#include <QtWidgets/QMessageBox>
#include <sstream>
#include "ui.h"
#include "ui_ui.h"

UI::UI(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::UI) {
    ui->setupUi(this);
}

UI::~UI() {
    delete ui;
}

void UI::askIfAppImageFileShouldBeOverridden() {
    std::ostringstream message;
    message << QObject::tr("AppImage with same filename has already been integrated.").toStdString() << std::endl
            << std::endl
            << QObject::tr("Do you wish to overwrite the existing AppImage?").toStdString() << std::endl;

    auto rv = QMessageBox::warning(
            this,
            QObject::tr("Warning"),
            QString::fromStdString(message.str()),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
    );

    if (rv == QMessageBox::Yes) {
        try {
            launcher->overrideAppImageIntegration();
            showCompletionPage();
        } catch (const std::runtime_error &ex) {
            notifyError(ex);
        }
    }
}

void UI::notifyError(const std::runtime_error &ex) {
    QMessageBox::critical(
            this,
            QObject::tr("Error"),
            ex.what()
    );
}

void UI::setLauncher(Launcher *launcher) {
    UI::launcher = launcher;
}

void UI::showIntegrationPage() {
    ui->stackedWidget->setCurrentWidget(ui->integrationPage);
    connect(ui->integrateButton, &QPushButton::released, this, &UI::handleIntegrationRequested);
    connect(ui->runButton, &QPushButton::released, this, &UI::handleExecutionRequested);
    show();
}

void UI::showCompletionPage() {
    ui->stackedWidget->setCurrentWidget(ui->completionPage);
    connect(ui->runButton2, &QPushButton::released, this, &UI::handleExecutionRequested);
    show();
}

void UI::handleIntegrationRequested() {
    try {
        launcher->integrateAppImage();
        showCompletionPage();
    } catch (const IntegrationFailed &ex) {
        notifyError(ex);
    } catch (const OverridingExistingAppImageFile &) {
        askIfAppImageFileShouldBeOverridden();
    }
}

void UI::handleExecutionRequested() {
    launcher->executeAppImage();
}