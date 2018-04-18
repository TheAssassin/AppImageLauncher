/* central file for utility functions */

// system headers
#include <string>

// library headers
#include <QString>

// makes an existing file executable
bool makeExecutable(const std::string& path);

// integrates an AppImage using a standard workflow used across all AppImageLauncher applications
bool integrateAppImage(const QString& pathToAppImage, const QString& pathToIntegratedAppImage);
