#ifndef UI_H
#define UI_H

#include <QDialog>
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
};

#endif // UI_H
