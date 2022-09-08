#ifndef APPIMAGELAUNCHER_INTEGRATION_DIALOG_H
#define APPIMAGELAUNCHER_INTEGRATION_DIALOG_H

// library includes
#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class IntegrationDialog; }
QT_END_NAMESPACE

class IntegrationDialog : public QDialog {
Q_OBJECT

public:
    enum ResultingAction {
        IntegrateAndRun,
        RunOnce,
        Settings
    };

    explicit IntegrationDialog(QString pathToAppImage, QString integratedAppImagesDestinationPath,
                               QWidget* parent = nullptr);

    ~IntegrationDialog() override;

    ResultingAction getResultAction() const;

protected:
    Q_SLOT void onPushButtonIntegrateAndRunReleased();

    Q_SLOT void onPushButtonRunOnceReleased();

    Q_SLOT void onPushButtonSettingsReleased();

    ResultingAction resultAction;

private:
    Ui::IntegrationDialog* ui;
    QString pathToAppImage;
    QString integratedAppImagesDestinationPath;

    void setIcon();

    void setMessage();
};

#endif //APPIMAGELAUNCHER_INTEGRATION_DIALOG_H
