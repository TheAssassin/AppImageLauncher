// libraries
#include <QtWidgets/QFileDialog>

// local
#include "settings_dialog.h"
#include "ui_settings_dialog.h"
#include "shared.h"

SettingsDialog::SettingsDialog(QWidget* parent) :
        QDialog(parent),
        ui(new Ui::SettingsDialog) {
    ui->setupUi(this);

    loadSettings();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onDialogAccepted);
    connect(ui->toolButtonChooseAppsDir, &QToolButton::released, this, &SettingsDialog::onChooseAppsDirClicked);
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::loadSettings() {
    settingsFile = getConfig();

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
    createConfigFile(ui->checkBoxAskMove->isChecked(),
                     ui->lineEditApplicationsDir->text(),
                     ui->checkBoxEnableDaemon->isChecked());

    settingsFile = getConfig();
}

void SettingsDialog::toggleDaemon() {
    // assumes defaults if config doesn't exist or lacks the related key(s)
    if (settingsFile) {
        if (settingsFile->value("AppImageLauncher/enable_daemon", false).toBool()) {
            system("systemctl --user enable appimagelauncherd.service");
            system("systemctl --user start  appimagelauncherd.service");
        } else {
            system("systemctl --user disable appimagelauncherd.service");
            system("systemctl --user stop    appimagelauncherd.service");
        }
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
