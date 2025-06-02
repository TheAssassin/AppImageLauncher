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
    void onAddDirectoryToWatchButtonClicked();
    void onRemoveDirectoryToWatchButtonClicked();
    void onDirectoryToWatchItemActivated(QListWidgetItem* item);

    void onAddExcludeDirButtonClicked();
    void onAddExcludeFileButtonClicked();
    void onRemoveExcludeButtonClicked();
    void onExcludeItemActivated(QListWidgetItem* item);

    void onDialogAccepted();

private:
    void loadSettings();

    void saveSettings();

    void toggleDaemon();

    void addDirectoryToWatchToListView(const QString& dirPath);
    void addExcludeToListView(const QString& fileOrDirPath);

    Ui::SettingsDialog* ui;
    QSettings* settingsFile;
};
