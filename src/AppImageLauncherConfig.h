#ifndef APPIMAGELAUNCHER_APPIMAGELAUNCHERCONFIG_H
#define APPIMAGELAUNCHER_APPIMAGELAUNCHERCONFIG_H


#include <QString>

class QSettings;

class AppImageLauncherConfig {
public:
    static QString getIntegratedAppImagesDir();

    static bool getDaemonEnabled();

    static QString getDefaultIntegrationDestination();

private:
    static QSettings* getSettings();
};


#endif //APPIMAGELAUNCHER_APPIMAGELAUNCHERCONFIG_H
