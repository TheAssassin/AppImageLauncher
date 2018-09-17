/* central file for utility functions */

// system headers
#include <string>
#include <memory>

// library headers
#include <QDir>
#include <QString>
#include <QSettings>

enum IntegrationState {
    INTEGRATION_FAILED = 0,
    INTEGRATION_SUCCESSFUL,
    INTEGRATION_ABORTED
};

// standard location for integrated AppImages
// currently hardcoded, can not be changed by users
static const auto DEFAULT_INTEGRATION_DESTINATION = QString(getenv("HOME")) + "/Applications/";

// load config file and return it
std::shared_ptr<QSettings> getConfig();

// checks whether AppImage has been integrated already
bool hasAlreadyBeenIntegrated(const QString& pathToAppImage);

// clean up old desktop files (and related resources, such as icons)
bool cleanUpOldDesktopIntegrationResources(bool verbose = false);
