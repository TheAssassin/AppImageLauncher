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

    ~SettingsDialog() override;

protected slots:
    void onChooseAppsDirClicked();
    void onAddDirectoryToWatchButtonClicked();

    void onDialogAccepted();

private:
    void loadSettings();

    void saveSettings();

    void toggleDaemon();

    void addDirectoryToWatchToListView(const QString& dirPath);

    Ui::SettingsDialog* ui;
    std::shared_ptr<QSettings> settingsFile;
};
