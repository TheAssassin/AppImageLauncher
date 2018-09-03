#ifndef UI_H
#define UI_H

// library includes
#include <QDialog>

// local includes
#include "AppImageDesktopIntegrationManager.h"
#include "Launcher.h"

namespace Ui {
    class UI;
}

class UI : public QDialog {
Q_OBJECT

    Launcher *launcher{nullptr};

public:

    explicit UI(QWidget *parent = 0);

    ~UI();

    void setLauncher(Launcher *launcher);

    void askIfAppImageFileShouldBeOverridden();

    void notifyError(const std::runtime_error &ex);

    void showIntegrationPage();

    void showCompletionPage();

protected slots:

    void handleIntegrationRequested();

    void handleExecutionRequested();

private:
    Ui::UI *ui;

    QString getLocalizedString(const nlohmann::json &info, const std::string &field) const;

    void setDefaultIcon() const;
};

#endif // UI_H
