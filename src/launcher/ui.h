#ifndef UI_H
#define UI_H

#include <QDialog>
#include "../AppImageDesktopIntegrationManager.h"
#include "Launcher.h"

namespace Ui {
    class UI;
}

/**
 * Provides a GUI for the Launcher class.
 */
class UI : public QDialog {
Q_OBJECT
    QSharedPointer<Launcher> launcher;
    QString sha512Checksum;

public:
    explicit UI(QWidget* parent = nullptr);

    ~UI() override;

    void setLauncher(QSharedPointer<Launcher> launcher);

    void askIfAppImageFileShouldBeOverridden();

    void notifyError(const std::runtime_error& ex);

    void showIntegrationPage();

    void showCompletionPage();

protected slots:
    void handleIntegrationRequested();

    void handleExecutionRequested();

    void toggleDetailsWidgetVisibility();

    void handleCopyCheckSumRequested();

private:
    Ui::UI* ui;

    QString getLocalizedString(const nlohmann::json& info, const std::string& field) const;

    void setDefaultIcon() const;

    void hideDetails() const;

    void showDetails() const;

    void fillLinksField(const nlohmann::json& info) const;

    void fillCategoriesField(const nlohmann::json& info) const;

    void fillLicenseField(const nlohmann::json& info) const;

    void setFileCorruptedWarningMessage() const;

    void fillFields();

    void fillIconField() const;

    void fillDescriptionField(const QString& abstract, const QString& description) const;
};

#endif // UI_H
