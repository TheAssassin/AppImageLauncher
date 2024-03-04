#pragma once

// system
#include <memory>

// libraries
#include <QDialog>
#include <QListWidgetItem>
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
    void onChooseSymlinkDirClicked();
    void onAddDirectoryToWatchButtonClicked();
    void onSymlinkAddButtonClicked();
    void onRemoveDirectoryToWatchButtonClicked();
    void onDirectoryToWatchItemActivated(QListWidgetItem* item);
    void onRemoveSymlinkButtonClicked();
    void onSymlinkItemActivated(QListWidgetItem* item);

    void onDialogAccepted();

private:
    void loadSettings();

    void saveSettings();

    void toggleDaemon();

    void addDirectoryToWatchToListView(const QString& dirPath);
    void addSymlinkToListView(const QString& symlinkName);

    Ui::SettingsDialog* ui;
    QSettings* settingsFile;
};
