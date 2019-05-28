#include <QtWidgets/QFileDialog>
#include "settings_dialog.h"
#include "ui_settings_dialog.h"

SettingsDialog::SettingsDialog(QWidget* parent) :
        QDialog(parent),
        ui(new Ui::SettingsDialog) {
    ui->setupUi(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onDialogAccepted);
    connect(ui->toolButtonChooseAppsDir, &QToolButton::released, this, &SettingsDialog::onChooseAppsDirClicked);
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::setSettingsFile(const std::shared_ptr<QSettings>& settingsFile) {
    SettingsDialog::settingsFile = settingsFile;

    loadSettings();
}

void SettingsDialog::loadSettings() {
    if (settingsFile) {
        ui->checkBoxEnableDaemon->setChecked(settingsFile->value("AppImageLauncher/enable_daemon", false).toBool());
        ui->checkBoxAskMove->setChecked(settingsFile->value("AppImageLauncher/ask_to_move", false).toBool());
        ui->lineEditApplicationsDir->setText(settingsFile->value("AppImageLauncher/destination").toString());
    }
}

void SettingsDialog::onDialogAccepted() {
    saveSettings();

    toggleDaemon();
}

void SettingsDialog::saveSettings() {
    settingsFile->setValue("AppImageLauncher/enable_daemon", ui->checkBoxEnableDaemon->isChecked());
    settingsFile->setValue("AppImageLauncher/ask_to_move", ui->checkBoxAskMove->isChecked());
    settingsFile->setValue("AppImageLauncher/destination", ui->lineEditApplicationsDir->text());
}

void SettingsDialog::toggleDaemon() {
    // assumes defaults if config doesn't exist or lacks the related key(s)
    if (settingsFile || !settingsFile->contains("AppImageLauncher/enable_daemon") ||
        settingsFile->value("AppImageLauncher/enable_daemon").toBool()) {
        system("systemctl --user enable appimagelauncherd.service");
        system("systemctl --user start  appimagelauncherd.service");
    } else {
        system("systemctl --user disable appimagelauncherd.service");
        system("systemctl --user stop    appimagelauncherd.service");
    }
}

void SettingsDialog::onChooseAppsDirClicked() {
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    fileDialog.setWindowTitle(tr("Select Applications directory"));
    fileDialog.setDirectory(QDir::home());
    if (fileDialog.exec()) {
        QString dirPath = fileDialog.selectedFiles().first();
        ui->lineEditApplicationsDir->setText(dirPath);
    }
}
