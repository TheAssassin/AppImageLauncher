#include <QStandardPaths>
#include <QSettings>
#include <QFileInfo>
#include <boost/shared_ptr.hpp>
#include "AppImageLauncherConfig.h"


QString AppImageLauncherConfig::getIntegratedAppImagesDir() {
    std::shared_ptr<QSettings> config(getSettings());

    static const QString keyName("AppImageLauncher/destination");
    return config->value(keyName, getDefaultIntegrationDestination()).toString();
}

QSettings *AppImageLauncherConfig::getSettings() {
    // calculate path to config file
    const auto configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    const auto configFilePath = configPath + "/appimagelauncher.cfg";

    // if the file does not exist, we'll just use the standard location
    // while in theory it would have been possible to just write the default location to the file, if we'd ever change
    // it again, we'd leave a lot of systems in the old state, and would have to write some complex code to resolve
    // the situation
    // therefore, the file is simply created, but left empty intentionally
    if (!QFileInfo::exists(configFilePath)) {
        QFile file(configFilePath);
        file.open(QIODevice::WriteOnly);
        file.write("[AppImageLauncher]\n"
                   "# destination = ~/Applications\n"
                   "# enable_daemon = true\n");
    }

    auto config = new QSettings(configFilePath, QSettings::IniFormat);
    return config;
}

QString AppImageLauncherConfig::getDefaultIntegrationDestination() {
    // standard location for integrated AppImages
    // currently hardcoded, can not be changed by users
    return QString(getenv("HOME")) + "/Applications/";
}
