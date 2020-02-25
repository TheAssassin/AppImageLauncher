// system includes
#include <stdexcept>

// library includes
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QDebug>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QLayout>
#include <QStyle>
#include <QUrl>

// local includes
#include "ui_first-run.h"
#include "shared.h"


class FirstRunDialog : public QDialog {
private:
    Ui::FirstRunDialog* firstRunDialog{};

    // stores custom destination dir for AppImages
    // if this is empty, the default value (defined in-code, might change over time) will be chosen
    // the default will not be written to the config file, hence we need a way to detect that state, and it's assumed
    // that when this string is empty, that's the case
    // we could also just compare this directory with the default value just before saving, but IMO it's more obvious
    // to the user that the "default" state is lost after having saved something with the "choose" button in a file
    // dialog
    QString destinationDir;

private Q_SLOTS:
    void resetDefaults() {
        firstRunDialog->askMoveCheckBox->setChecked(true);

        destinationDir = "";
        updateDestinationDirLabel();
    }

    void handleButtonClicked(QAbstractButton* button) {
        if (button == firstRunDialog->buttonBox->button(QDialogButtonBox::RestoreDefaults)) {
            qDebug() << "restore defaults";
            resetDefaults();
        } else if (button == firstRunDialog->buttonBox->button(QDialogButtonBox::Help)) {
            qDebug() << "help";
            QDesktopServices::openUrl(QUrl("https://github.com/TheAssassin/AppImageLauncher/wiki/First-run"));

        } else {
            qDebug() << "unknown button clicked" << button;
        }
    }

    void handleAskMoveCheckBoxStateChange(int state) {
        qDebug() << "new ask move check box state" << state;

        // this alone unfortunately doesn't do the trick...
        for (auto* layout : {
            static_cast<QLayout*>(firstRunDialog->destDirVertLayout),
            static_cast<QLayout*>(firstRunDialog->destDirHorLayout),
        }) {
            layout->setEnabled(state > 0);
        }

        // have to also manually enable/disable all the
        for (auto* label : {
            static_cast<QWidget*>(firstRunDialog->destinationDirDescLabel),
            static_cast<QWidget*>(firstRunDialog->destinationDirLabel),
            static_cast<QWidget*>(firstRunDialog->customizeIntegrationDirButton),
        }) {
            label->setEnabled(state > 0);
        }
    }

    void handleCustomizeIntegrationDirButtonClicked(bool checked = false) {
        (void) checked;

        auto oldDir = destinationDir;
        if (oldDir.isEmpty())
            oldDir = integratedAppImagesDestination().absolutePath();

        auto newDir = QFileDialog::getExistingDirectory(this, tr("Choose integration destination dir"), oldDir);

        // the call above returns an empty string if the user aborts the dialog
        if (!newDir.isEmpty()) {
            destinationDir = newDir;
        }

        // updating never is a bad idea
        updateDestinationDirLabel();
    }

private:
    void updateDestinationDirLabel() {
        QString text = destinationDir;

        // fallback to default
        if (text.isEmpty())
            text = integratedAppImagesDestination().absolutePath() + " " + tr("(default)");

        firstRunDialog->destinationDirLabel->setText(text);
    }

    void initUi() {
        firstRunDialog = new Ui::FirstRunDialog;
        // setupUi will modify this dialog so that it looks just like what we designed in Qt Designer
        firstRunDialog->setupUi(this);

        // set up logo in a QLabel
        firstRunDialog->logoLabel->setText("");
        auto pixmap = QPixmap::fromImage(QImage(":/AppImageLauncher.svg")).scaled(QSize(128,128),
                Qt::KeepAspectRatio, Qt::SmoothTransformation
        );
        firstRunDialog->logoLabel->setPixmap(pixmap);

        // setting icon in Qt Designer doesn't seem to work
        firstRunDialog->customizeIntegrationDirButton->setIcon(this->style()->standardIcon(QStyle::SP_DirIcon));

        // reset defaults
        resetDefaults();

        // set up all connections
        connect(firstRunDialog->buttonBox, &QDialogButtonBox::clicked, this, &FirstRunDialog::handleButtonClicked);
        connect(firstRunDialog->askMoveCheckBox, &QCheckBox::stateChanged, this, &FirstRunDialog::handleAskMoveCheckBoxStateChange);

        connect(firstRunDialog->customizeIntegrationDirButton, &QPushButton::clicked, this, &FirstRunDialog::handleCustomizeIntegrationDirButtonClicked);
    }

public:
    FirstRunDialog() {
        initUi();
    }

    void writeConfigFile() {
        bool askToMove = firstRunDialog->askMoveCheckBox->checkState() == Qt::Checked;
        createConfigFile(askToMove ? 1 : 0, destinationDir, -1);
    }
};


void showFirstRunDialog() {
    auto dialog = new FirstRunDialog;

    setUpFallbackIconPaths(dialog);

    auto rv = dialog->exec();

    if (rv <= 0) {
        QApplication::exit(3);
        exit(3);
    }

    dialog->writeConfigFile();
}
