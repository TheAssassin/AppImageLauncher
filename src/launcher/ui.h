#ifndef UI_H
#define UI_H

#include <QObject>
#include "AppImageDesktopIntegrationManager.h"

class UI : public QObject {
Q_OBJECT
public:
    UI();

    void askIfAppImageFileShouldBeOverridden();

    void alertIntegrationFailed(const IntegrationFailed &ex);
signals:

    void overrideAppImageFile();

    void dontOverrideAppImageFile();

    void alertIntegrationFailedCompleted();
};

#endif // UI_H