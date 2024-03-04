// libraries
#include <QDebug>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QStandardPaths>
#include <QInputDialog>
#include <QProcess>

// local
#include "settings_dialog.h"
#include "ui_settings_dialog.h"
#include "shared.h"

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), ui(new Ui::SettingsDialog),
                                                  settingsFile(getConfig(this)) {
    ui->setupUi(this);

    ui->applicationsDirLineEdit->setPlaceholderText(integratedAppImagesDestination().absolutePath());

    loadSettings();

// cosmetic changes in lite mode
#ifdef BUILD_LITE
    ui->daemonIsEnabledCheckBox->setChecked(true);
    ui->daemonIsEnabledCheckBox->setEnabled(false);

    ui->askMoveCheckBox->setChecked(false);
    ui->askMoveCheckBox->setEnabled(false);
#endif

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onDialogAccepted);
    connect(ui->chooseAppsDirToolButton, &QToolButton::released, this, &SettingsDialog::onChooseAppsDirClicked);
    connect(ui->chooseSymlinkDirButton, &QToolButton::released, this, &SettingsDialog::onChooseSymlinkDirClicked);
    connect(ui->additionalDirsAddButton, &QToolButton::released, this, &SettingsDialog::onAddDirectoryToWatchButtonClicked);
    connect(ui->additionalDirsRemoveButton, &QToolButton::released, this, &SettingsDialog::onRemoveDirectoryToWatchButtonClicked);
    connect(ui->additionalDirsListWidget, &QListWidget::itemActivated, this, &SettingsDialog::onDirectoryToWatchItemActivated);
    connect(ui->additionalDirsListWidget, &QListWidget::itemClicked, this, &SettingsDialog::onDirectoryToWatchItemActivated);
    connect(ui->symlinkList, &QListWidget::itemActivated, this, &SettingsDialog::onSymlinkItemActivated);
    connect(ui->symlinkList, &QListWidget::itemClicked, this, &SettingsDialog::onSymlinkItemActivated);
    connect(ui->addLinkButton, &QToolButton::released, this, &SettingsDialog::onSymlinkAddButtonClicked);
    connect(ui->removeLinkButton, &QToolButton::released, this, &SettingsDialog::onRemoveSymlinkButtonClicked);

    QStringList availableFeatures;

#ifdef ENABLE_UPDATE_HELPER
    availableFeatures << "<span style='color: green;'>✔</span> " + tr("updater available for AppImages supporting AppImageUpdate");
#else
    availableFeatures << "<span style='color: red;'>🞬</span> " + tr("updater unavailable");
#endif

#ifdef BUILD_LITE
    availableFeatures << "<br /><br />"
                      << tr("<strong>Note: this is an AppImageLauncher Lite build, only supports a limited set of features</strong><br />"
                            "Please install the full version via the provided native packages to enjoy the full AppImageLauncher experience");
#endif

    ui->featuresLabel->setText(availableFeatures.join('\n'));

    // no matter what tab was selected when saving in Qt designer, we want to start up with the first tab
    ui->tabWidget->setCurrentWidget(ui->launcherTab);
}

SettingsDialog::~SettingsDialog() {
    delete ui;
}

void SettingsDialog::addDirectoryToWatchToListView(const QString& dirPath) {
    // empty paths are not permitted
    if (dirPath.isEmpty())
        return;

    const QDir dir(dirPath);

    // we don't want to redundantly add the main integration directory
    if (dir == integratedAppImagesDestination())
        return;

    QIcon icon;

    auto findIcon = [](const std::initializer_list<QString>& names) {
        for (const auto& i : names) {
            auto icon = QIcon::fromTheme(i, loadIconWithFallback(i));

            if (!icon.isNull())
                return icon;
        }

        return QIcon{};
    };

    if (dir.exists()) {
        icon = findIcon({"folder"});
    } else {
        // TODO: search for more meaningful icon, "remove" doesn't really show the directory is missing
        icon = findIcon({"remove"});
    }

    if (icon.isNull()) {
        qDebug() << "item icon unavailable, using fallback";
    }

    auto* item = new QListWidgetItem(icon, dirPath);
    ui->additionalDirsListWidget->addItem(item);
}

void SettingsDialog::addSymlinkToListView(const QString& symlinkName) {
    // empty paths are not permitted
    if (symlinkName.isEmpty())
        return;

    // Doesn't add duplicated items.
    QList currentItems = ui->symlinkList->findItems(symlinkName, Qt::MatchFlag::MatchExactly);
    if(!currentItems.empty() && currentItems.first() != nullptr) {
        return;
    }

    const QFile file(symlinkName);

    QIcon icon;

    auto findIcon = [](const std::initializer_list<QString>& names) {
        for (const auto& i : names) {
            auto icon = QIcon::fromTheme(i, loadIconWithFallback(i));

            if (!icon.isNull())
                return icon;
        }

        return QIcon{};
    };

    if (file.exists()) {
        icon = findIcon({"emblem-symbolic-link"});
    } else {
        icon = findIcon({"emblem-unreadable"});
    }

    if (icon.isNull()) {
        qDebug() << "item icon unavailable, using fallback";
    }

    auto* item = new QListWidgetItem(icon, symlinkName);
    ui->symlinkList->addItem(item);
}

void SettingsDialog::loadSettings() {
    const auto daemonIsEnabled = settingsFile->value("AppImageLauncher/enable_daemon", "true").toBool();
    const auto askMoveChecked = settingsFile->value("AppImageLauncher/ask_to_move", "true").toBool();

    if (settingsFile) {
        ui->daemonIsEnabledCheckBox->setChecked(daemonIsEnabled);
        ui->askMoveCheckBox->setChecked(askMoveChecked);
        ui->applicationsDirLineEdit->setText(settingsFile->value("AppImageLauncher/destination").toString());
        ui->symlinkLineEdit->setText(integratedSymlinkDestination().absolutePath());

        const auto additionalDirsPath = settingsFile->value("appimagelauncherd/additional_directories_to_watch", "").toString();
        for (const auto& dirPath : additionalDirsPath.split(":")) {
            addDirectoryToWatchToListView(dirPath);
        }

        const auto symlinks = settingsFile->value("AppImageLauncher/symlinks", "").toString();
        for (const auto& linkPath : symlinks.split(":")) {
            QFile file(linkPath);

            if(file.exists()) {
                addSymlinkToListView(linkPath);
            }
        }
    }
}

void SettingsDialog::onDialogAccepted() {
    saveSettings();
    toggleDaemon();
}

void SettingsDialog::saveSettings() {
    QStringList additionalDirsToWatch;
    QStringList symlinks;

    {
        QListWidgetItem* currentItem;

        for (int i = 0; (currentItem = ui->additionalDirsListWidget->item(i)) != nullptr; ++i) {
            additionalDirsToWatch << currentItem->text();
        }
    }

    {
        QListWidgetItem* currentItem;

        for (int i = 0; (currentItem = ui->symlinkList->item(i)) != nullptr; ++i) {
            symlinks << currentItem->text();
        }
    }

    // temporary workaround to fill in the monitorMountedFilesystems with the same value it had in the old settings
    // this is supposed to support the option while hiding it in the settings
    int monitorMountedFilesystems = -1;
    {
        const auto oldSettings = getConfig();

        static constexpr auto oldKey = "appimagelauncherd/monitor_mounted_filesystems";

        // getConfig might return a null pointer if the config file doesn't exist
        // we have to handle this, obviously
        if (oldSettings != nullptr && oldSettings->contains(oldKey)) {
            const auto oldValue = oldSettings->value(oldKey).toBool();
            monitorMountedFilesystems = oldValue ? 1 : 0;
        }
    }

    createConfigFile(ui->askMoveCheckBox->isChecked(),
                     ui->applicationsDirLineEdit->text(),
                     ui->symlinkLineEdit->text(),
                     ui->daemonIsEnabledCheckBox->isChecked(),
                     additionalDirsToWatch,
                     symlinks,
                     monitorMountedFilesystems);

    // reload settings
    loadSettings();
}

void SettingsDialog::toggleDaemon() {
    // assumes defaults if config doesn't exist or lacks the related key(s)
    if (settingsFile) {
        if (settingsFile->value("AppImageLauncher/enable_daemon", "true").toBool()) {
            system("systemctl --user enable  appimagelauncherd.service");
            // we want to actually restart the service to apply the new configuration
            system("systemctl --user restart appimagelauncherd.service");
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
    fileDialog.setDirectory(integratedAppImagesDestination().absolutePath());

    // Gtk+ >= 3 segfaults when trying to use the native dialog, therefore we need to enforce the Qt one
    // See #218 for more information
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);

    if (fileDialog.exec()) {
        QString dirPath = fileDialog.selectedFiles().first();
        ui->applicationsDirLineEdit->setText(dirPath);
    }
}

void SettingsDialog::onChooseSymlinkDirClicked() {
    QFileDialog fileDialog(this);

    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    fileDialog.setWindowTitle(tr("Select Symlinks directory"));
    fileDialog.setDirectory(integratedSymlinkDestination().absolutePath());

    // Gtk+ >= 3 segfaults when trying to use the native dialog, therefore we need to enforce the Qt one
    // See #218 for more information
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);

    if (fileDialog.exec()) {
        QString dirPath = fileDialog.selectedFiles().first();
        ui->symlinkLineEdit->setText(dirPath);
    }
}

void SettingsDialog::onAddDirectoryToWatchButtonClicked() {
    QFileDialog fileDialog(this);

    fileDialog.setFileMode(QFileDialog::DirectoryOnly);
    fileDialog.setWindowTitle(tr("Select additional directory to watch"));
    fileDialog.setDirectory(QStandardPaths::locate(QStandardPaths::HomeLocation, ".", QStandardPaths::LocateDirectory));

    // Gtk+ >= 3 segfaults when trying to use the native dialog, therefore we need to enforce the Qt one
    // See #218 for more information
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);

    if (fileDialog.exec()) {
        QString dirPath = fileDialog.selectedFiles().first();
        addDirectoryToWatchToListView(dirPath);
    }
}

void SettingsDialog::onSymlinkAddButtonClicked() {
    QFileDialog fileDialog(this);

    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.selectMimeTypeFilter("*.AppImage");
    fileDialog.setDirectory(integratedAppImagesDestination().absolutePath());

    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);

    if (fileDialog.exec()) {
        QString filePath = fileDialog.selectedFiles().first();
        QInputDialog symlinkNameDialog(this);

        symlinkNameDialog.setInputMode(QInputDialog::TextInput);
        symlinkNameDialog.setOkButtonText("Add");
        symlinkNameDialog.setLabelText("Select the symlink name");
        if (symlinkNameDialog.exec()) {
            QString linkName = symlinkNameDialog.textValue();
            QString fullSymlinkPath = integratedSymlinkDestination().absolutePath() + "/" + linkName;
            system(QString("ln -s %1 %2").arg(filePath, fullSymlinkPath).toStdString().c_str());
            addSymlinkToListView(fullSymlinkPath);

            // As the actions is taken right way, it should apply the settings
            // may create another function to update just this entry
            saveSettings();
        }
    }
}

void SettingsDialog::onRemoveDirectoryToWatchButtonClicked() {
    auto* widget = ui->additionalDirsListWidget;

    auto* currentItem = widget->currentItem();

    if (currentItem == nullptr)
        return;

    const auto index = widget->row(currentItem);

    // after taking it, we have to delete it ourselves, Qt docs say
    auto deletedItem = widget->takeItem(index);
    delete deletedItem;

    // we should deactivate the remove button once the last item is gone
    if (widget->item(0) == nullptr) {
        ui->additionalDirsRemoveButton->setEnabled(false);
    }
}

void SettingsDialog::onRemoveSymlinkButtonClicked() {
    auto* widget = ui->symlinkList;

    auto* currentItem = widget->currentItem();

    if (currentItem == nullptr)
        return;

    const auto index = widget->row(currentItem);

    system(QString("rm %1").arg(currentItem->text()).toStdString().c_str());

    // after taking it, we have to delete it ourselves, Qt docs say
    auto deletedItem = widget->takeItem(index);
    delete deletedItem;

    // we should deactivate the remove button once the last item is gone
    if (widget->item(0) == nullptr) {
        ui->removeLinkButton->setEnabled(false);
    }
}

void SettingsDialog::onDirectoryToWatchItemActivated(QListWidgetItem* item) {
    // we activate the button based on whether there's an item selected
    ui->additionalDirsRemoveButton->setEnabled(item != nullptr);
}

void SettingsDialog::onSymlinkItemActivated(QListWidgetItem* item) {
    // we activate the button based on whether there's an item selected
    ui->removeLinkButton->setEnabled(item != nullptr);
}
