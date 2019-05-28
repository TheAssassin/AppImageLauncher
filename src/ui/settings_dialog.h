#pragma once
// system
#include <memory>

// libraries
#include <QDialog>
#include <QSettings>


namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog {
Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    void setSettingsFile(const std::shared_ptr<QSettings>& settingsFile);

    ~SettingsDialog() override;

protected slots:
    void onChooseAppsDirClicked();

    void onDialogAccepted();

private:
    void loadSettings();

    void saveSettings();

    void toggleDaemon();

    Ui::SettingsDialog* ui;
    std::shared_ptr<QSettings> settingsFile;
};
