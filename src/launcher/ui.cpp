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

    ui->labelIcon->setText("");
}

void UI::setDefaultIcon() const {
    auto appimageIcon = QIcon::fromTheme("appimage");
    if (appimageIcon.isNull())
        appimageIcon = QIcon::fromTheme("application-x-executable");
    ui->labelIcon->setPixmap(appimageIcon.pixmap(64, 64));
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
        setAppImageInfo();
    } catch (const AppImageFileNotExists &ex) {
        notifyError(ex);
    } catch (const AppImageFilePathNotSet &) {
        qWarning() << "Launcher instance not initialized properly.";
    } catch (const InvalidAppImageFile &ex) {
        setFileCorruptedWarningMessage();
    }

    ui->stackedWidget->setCurrentWidget(ui->integrationPage);
    connect(ui->integrateButton, &QPushButton::released, this, &UI::handleIntegrationRequested);
    connect(ui->runButton, &QPushButton::released, this, &UI::handleExecutionRequested);
    connect(ui->detailsButton, &QPushButton::released, this, &UI::toggleDetailsWidgetVisibility);
    connect(ui->copyCheckSumButton, &QPushButton::released, this, &UI::handleCopyCheckSumRequested);
    show();
}

void UI::setAppImageInfo() {
    auto info = launcher->getAppImageInfo();

    QString name = getLocalizedString(info, "name");
    QString abstract = getLocalizedString(info, "abstract");
    QString description = getLocalizedString(info, "description");

    auto type = info["file"]["type"].get<int>();
    shaChecksum = QString::fromStdString(info["file"]["sha512checksum"].get<std::string>());
    auto arch = info["file"]["architecture"];

    setLicense(info);
    setCategories(info);

    ui->labelType->setNum(type);
    ui->labelArchitecture->setText(QString::fromStdString(arch));

    setLinks(info);

    if (description.isEmpty())
        ui->labelDescription->setText(abstract);
    else
        ui->labelDescription->setText(description);

    ui->labelName->setText(name);
    ui->labelAbstract->setText(abstract);

    auto icon = launcher->getAppImageIcon();
    if (icon.isNull())
        setDefaultIcon();
    else
        ui->labelIcon->setPixmap(icon.pixmap(64, 64));

    hideDetails();
}

void UI::setFileCorruptedWarningMessage() const {
    ui->labelIcon->setVisible(false);
    ui->labelName->setVisible(false);
    ui->labelLicense->setVisible(false);
    ui->labelCategories->setVisible(false);
    ui->labelLinks->setVisible(false);
    ui->detailsButton->setVisible(false);
    ui->labelAbstract->setText(
            tr("<p>Unable to read the AppImage information. The file is <b>corrupt</b> or <b>not properly "
               "built</b>. Notify to the application developers about this issue.</p>"
               "<p>This may avoid the proper execution of the <i>Integrate</i> and <i>Run</i> functionalities</p>"
               "<b>Use it at your own risk.</b>"));
    ui->labelAbstract->setMargin(0);
    hideDetails();
}

void UI::setLicense(const nlohmann::json &info) const {
    QString licenseText;
    if (info.find("license") != info.end()) {
        auto license = info["license"];
        if (license.find("id") != license.end())
            licenseText = QString::fromStdString(license["id"]);
    }
    if (licenseText.isEmpty())
        ui->labelLicense->setVisible(false);
    else
        ui->labelLicense->setText(licenseText);
}

void UI::setCategories(const nlohmann::json &info) const {
    QStringList categories;
    if (info.find("categories") != info.end()) {
        for (const auto &item: info["categories"]) {
            const auto text = QString::fromStdString(item.get<std::string>());
            // Ignore custom categories
            if (!text.startsWith("X", Qt::CaseInsensitive))
                categories << QObject::tr(text.toStdString().c_str());
        }
    }

    if (categories.isEmpty())
        ui->labelCategories->setVisible(false);
    else
        ui->labelCategories->setText(categories.join(" "));
}

void UI::setLinks(const nlohmann::json &info) const {
    QStringList links;
    if (info.find("links") != info.end()) {
        for (auto it = info["links"].begin(); it != info["links"].end(); ++it) {
            const auto linkName = QString::fromStdString(it.key());
            const auto linkAddr = QString::fromStdString(it.value());
            const auto trLinkName = QObject::tr(linkName.toStdString().c_str());
            links << QString("<a href=%2>%1</a>").arg(trLinkName).arg(linkAddr);
        }
    }

    if (links.isEmpty())
        ui->labelLinks->setVisible(false);
    else
        ui->labelLinks->setText(links.join(" "));
}

QString UI::getLocalizedString(const nlohmann::json &info, const std::string &field) const {
    QString value;
    auto locale = QLocale::system().name().toStdString();
    if (info.find(field) != info.end()) {
        if (info[field].find(locale) != info[field].end())
            value = QString::fromStdString(info[field][locale].get<std::string>());
        else if (info[field].find("default") != info[field].end())
            value = QString::fromStdString(info[field]["default"].get<std::string>());
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
    try {
        launcher->executeAppImage();
    }
    catch (const std::runtime_error &error) {
        notifyError(error);
    }
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
    clipboard->setText(shaChecksum);
    QToolTip::showText(ui->copyCheckSumButton->mapToGlobal(QPoint(20, 0)), tr("Copied to clipboard"));
}
