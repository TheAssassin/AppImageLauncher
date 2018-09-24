#ifndef UI_H
#define UI_H

#include <QDialog>
#include "../AppImageDesktopIntegrationManager.h"
#include "Launcher.h"

namespace Ui {
    class UI;
}

class UI : public QDialog {
Q_OBJECT

    Launcher* launcher{nullptr};
    QString shaChecksum;

public:

    explicit UI(QWidget* parent = 0);

    ~UI();

    void setLauncher(Launcher* launcher);

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

    void setLinks(const nlohmann::json& info) const;

    void hideDetails() const;

    void showDetails() const;

    void setCategories(const nlohmann::json& info) const;

    void setLicense(const nlohmann::json& info) const;

    void setFileCorruptedWarningMessage() const;

    void setAppImageInfo();
};

#endif // UI_H
