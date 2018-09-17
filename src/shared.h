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

// load config file and return it
std::shared_ptr<QSettings> getConfig();
