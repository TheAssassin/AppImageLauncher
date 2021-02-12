#ifndef APPIMAGELAUNCHER_INTEGRATION_DIALOG_H
#define APPIMAGELAUNCHER_INTEGRATION_DIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class IntegrationDialog; }
QT_END_NAMESPACE

class IntegrationDialog : public QDialog {
Q_OBJECT

public:
    explicit IntegrationDialog(QWidget* parent = nullptr);

    ~IntegrationDialog() override;

private:
    Ui::IntegrationDialog* ui;
};

#endif //APPIMAGELAUNCHER_INTEGRATION_DIALOG_H
