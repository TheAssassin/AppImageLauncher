// system includes
#include <iostream>
#include <sstream>

// library includes
#include <QApplication>
#include <QCheckBox>
#include <QCommandLineParser>
#include <QFile>
#include <QLibraryInfo>
#include <QMessageBox>
#include <QPushButton>
#include <QTranslator>
#include <QThread>
extern "C" {
    #include <appimage/appimage.h>
}
#include <appimage/update/qt-ui.h>

// local includes
#include "shared.h"
#include "translationmanager.h"
#include "ui_update.h"

using namespace appimage::update::qt;

class UpdateDialog : public QDialog {
    Q_OBJECT

public:
    explicit UpdateDialog(const QString& pathToAppImage) : _pathToAppImage(pathToAppImage), _ui(new Ui::UpdateDialog), _updater(new QtUpdater(pathToAppImage)) {
        // configure UI
        _ui->setupUi(this);

        _ui->stackedWidget->setCurrentIndex(0);

        // these three calls are needed to give the QQuickWidget the same background color as its parent window
        _ui->spinnerQuickWidget->setAttribute(Qt::WA_AlwaysStackOnTop);
        _ui->spinnerQuickWidget->setAttribute(Qt::WA_TranslucentBackground);
        _ui->spinnerQuickWidget->setClearColor(Qt::transparent);

        _ui->spinnerQuickWidget->setSource(QUrl::fromLocalFile(":/update_spinner.qml"));

        // can't add the widget to the page directly in the .ui file since the constructor needs parameters
        _ui->updaterPage->layout()->addWidget(_updater);

        // make sure the QDialog resizes with the spoiler
        layout()->setSizeConstraint(QLayout::SetFixedSize);

        // make sure that when the embedded dialog closes, the parent dialog closes, too
        connect(_updater, &QDialog::finished, this, &QDialog::done);

        // set up updater
        _updater->enableRunUpdatedAppImageButton(false);

        connect(
            _updater,
            &QtUpdater::newStatusMessage,
            this,
            [this](const std::string& newMessage) {
                if (_updaterStatusMessages.tellp() > 0)
                    _updaterStatusMessages << std::endl;

                _updaterStatusMessages << newMessage;
            }
        );

        connect(this, &UpdateDialog::updateCheckFinished, this, [this](int updateCheckResult) {
            switch (updateCheckResult) {
                case 1:
                    clearStatusMessages();
                    _ui->stackedWidget->setCurrentWidget(_ui->updaterPage);
                    _updater->update();
                    return;
                case 0: {
                    _ui->errorTitleLabel->setText(tr("No updates found"));
                    _ui->errorMessageLabel->setText(tr("Could not find updates for AppImage %1").arg(_pathToAppImage));
                    connect(_ui->errorButtonBox, &QDialogButtonBox::clicked, this, &QDialog::accept);
                    break;
                }
                case -1: {
                    _ui->errorTitleLabel->setText(tr("No update information found"));
                    _ui->errorMessageLabel->setText(
                        tr("Could not find update information in AppImage:\n%1"
                           "\n"
                           "\n"
                           "The AppImage doesn't support updating. Please ask the authors to embed "
                           "update information to allow for easy updating."
                        ).arg(_pathToAppImage)
                    );
                    connect(_ui->errorButtonBox, &QDialogButtonBox::clicked, this, &QDialog::reject);
                    break;
                }
                default: {
                    _ui->errorTitleLabel->setText(tr("Update check failed"));
                    _ui->errorMessageLabel->setText(tr("Failed to check for updates:\n%1").arg(_pathToAppImage));
                    connect(_ui->errorButtonBox, &QDialogButtonBox::clicked, this, &QDialog::reject);
                    break;
                }
            }

            _ui->stackedWidget->setCurrentWidget(_ui->errorPage);
            qCritical() << statusMessages();
        });

        asyncCheckForUpdate();
    }

    ~UpdateDialog() override {
        // TODO: parenting in libappimageupdate
        delete _updater;
    }

    QString statusMessages() const {
        return QString::fromStdString(_updaterStatusMessages.str());
    }

    void clearStatusMessages() {
        _updaterStatusMessages.clear();
    }

    void asyncCheckForUpdate() {
        // TODO: implement an async update check in libappimageupdate
        auto* thread = QThread::create([this]() {
            auto updateCheckResult = _updater->checkForUpdates();
            emit updateCheckFinished(updateCheckResult);
        });
        thread->start();
    }

signals:
    void updateCheckFinished(int checkResult);

private:
    const QString _pathToAppImage;
    Ui::UpdateDialog* _ui;
    QtUpdater *_updater;
    std::ostringstream _updaterStatusMessages;
};

#include "update_main.moc"


int main(int argc, char** argv) {
    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("Updates AppImages after desktop integration, for use by Linux distributions"));

    QApplication app(argc, argv);
    QApplication::setApplicationDisplayName(QObject::tr("AppImageLauncher update", "update helper app name"));
    QApplication::setWindowIcon(QIcon(":/AppImageLauncher.svg"));

    std::ostringstream version;
    version << "version " << APPIMAGELAUNCHER_VERSION << " "
            << "(git commit " << APPIMAGELAUNCHER_GIT_COMMIT << "), built on "
            << APPIMAGELAUNCHER_BUILD_DATE;
    QApplication::setApplicationVersion(QString::fromStdString(version.str()));

    // install translations
    TranslationManager translationManager(app);

    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);

    parser.addPositionalArgument("path", "Path to AppImage", "<path>");

    if (parser.positionalArguments().empty()) {
        parser.showHelp(1);
    }

    const auto pathToAppImage = parser.positionalArguments().first();

    auto criticalUpdaterError = [](const QString& message) {
        QMessageBox::critical(nullptr, "Error", message);
    };

    if (!QFile(pathToAppImage).exists()) {
        criticalUpdaterError(QString::fromStdString(QObject::tr("Error: no such file or directory: %1").arg(pathToAppImage).toStdString()));
        return 1;
    }

    const auto type = appimage_get_type(pathToAppImage.toStdString().c_str(), false);

    if (type <= 0 || type > 2) {
        criticalUpdaterError(QObject::tr("Not an AppImage: %1").arg(pathToAppImage));
        return 1;
    }

    auto dialog = new UpdateDialog(pathToAppImage);
    dialog->show();

    return QApplication::exec();
}
