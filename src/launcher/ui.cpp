#include <QMessageBox>
#include <QClipboard>
#include <QToolTip>
#include <QIcon>
#include <QDebug>
#include <sstream>
#include "ui.h"
#include "ui_ui.h"

UI::UI(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::UI) {
    ui->setupUi(this);

    ui->iconLabel->setText("");
}

void UI::setDefaultIcon() const {
    auto appimageIcon = QIcon::fromTheme("appimage");
    if (appimageIcon.isNull())
        appimageIcon = QIcon::fromTheme("application-x-executable");
    ui->iconLabel->setPixmap(appimageIcon.pixmap(64, 64));
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
    try {
        auto info = launcher->getAppImageInfo();
        QString name = getLocalizedString(info, "name");
        QString abstract = getLocalizedString(info, "abstract");
        QString description = getLocalizedString(info, "description");

        auto type = info["file"]["type"].get<int>();
        auto sha512checksum = info["file"]["sha512checksum"];
        auto arch = info["file"]["architecture"];

        ui->labelType->setNum(type);
        ui->labelCheckSum->setText(QString::fromStdString(sha512checksum));
        ui->labelArchitecture->setText(QString::fromStdString(arch));

        setWebsiteLink(info);

        ui->labelDescription->setText(description);
        ui->labelName->setText(name);
        ui->labelAbstract->setText(abstract);

        auto icon = launcher->getAppImageIcon();
        if (icon.isNull())
            setDefaultIcon();
        else
            ui->iconLabel->setPixmap(icon.pixmap(64, 64));

        hideDetails();
    } catch (const AppImageFileNotExists &ex) {
        notifyError(ex);
    } catch (const AppImageFilePathNotSet &) {
        qWarning() << "Launcher instance not initialized properly.";
    }

    ui->stackedWidget->setCurrentWidget(ui->integrationPage);
    connect(ui->integrateButton, &QPushButton::released, this, &UI::handleIntegrationRequested);
    connect(ui->runButton, &QPushButton::released, this, &UI::handleExecutionRequested);
    connect(ui->detailsButton, &QPushButton::released, this, &UI::toggleDetailsWidgetVisibility);
    connect(ui->copyCheckSumButton, &QPushButton::released, this, &UI::handleCopyCheckSumRequested);
    show();
}

void UI::setWebsiteLink(const nlohmann::json &info) const {
    if (info.find("links") != info.end() && info["links"].find("homepage") != info["links"].end()) {
        auto value = info["links"]["homepage"].get<std::__cxx11::string>();
        QString homepageLink = QString::fromStdString(value);
        ui->labelWebsite->setText(QString("<a href=\"%1\">Website</a>").arg(homepageLink));
        ui->labelWebsite->setVisible(true);
    } else
        ui->labelWebsite->setVisible(false);
}

QString UI::getLocalizedString(const nlohmann::json &info, const std::string &field) const {
    QString value;
    auto locale = QLocale::system().name().toStdString();
    if (info.find(field) != info.end()) {
        if (info[field].find(locale) != info[field].end())
            value = QString::fromStdString(info[field][locale].get<std::__cxx11::string>());
        else if (info[field].find("default") != info[field].end())
            value = QString::fromStdString(info[field]["default"].get<std::__cxx11::string>());
    }
    return value;
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

void UI::toggleDetailsWidgetVisibility() {
    if (ui->widgetDetails->isVisible())
        hideDetails();
    else
        showDetails();
}

void UI::showDetails() const {
    ui->labelAbstract->setVisible(false);
    ui->widgetDetails->setVisible(true);
    window()->resize(600, 400);
}

void UI::hideDetails() const {
    ui->labelAbstract->setVisible(true);
    ui->widgetDetails->setVisible(false);
    window()->resize(340, 280);
}

void UI::handleCopyCheckSumRequested() {
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setText(ui->labelCheckSum->text());
    QToolTip::showText( ui->copyCheckSumButton->mapToGlobal( QPoint( 20, 0 ) ), tr("Copied to clipboard"));
}
