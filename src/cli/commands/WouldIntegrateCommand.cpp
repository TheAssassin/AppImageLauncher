// library headers
#include <QFileInfo>

extern "C" {
#include <appimage/appimage.h>
}

// local headers
#include "WouldIntegrateCommand.h"
#include "exceptions.h"
#include "shared.h"
#include "logging.h"

namespace appimagelauncher {
    namespace cli {
        namespace commands {
            void WouldIntegrateCommand::exec(QList<QString> arguments) {
                if (arguments.empty()) {
                    throw InvalidArgumentsError("No AppImages passed on commandline");
                }

                // make sure all AppImages exist on disk before further processing
                for (auto& path : arguments) {
                    if (!QFileInfo(path).exists()) {
                        throw UsageError("could not find file " + path);
                    }

                    // make path absolute
                    // that will just prevent mistakes in libappimage and shared etc.
                    // (stuff like TryExec keys etc. being set to paths relative to CWD when running the command , ...)
                    path = QFileInfo(path).absoluteFilePath();
                }

                for (const auto& pathToAppImage : arguments) {
                    qout() << "Checking whether " << pathToAppImage << " should be integrated" << endl;

                    if (!QFileInfo(pathToAppImage).isFile()) {
                        qerr() << "Warning: Not a file, skipping: " << pathToAppImage << endl;
                        continue;
                    }

                    if (!isAppImage(pathToAppImage)) {
                        qerr() << "Warning: Not an AppImage, skipping: " << pathToAppImage << endl;
                        continue;
                    }

                    // TODO: refactor into a function that, e.g., returns an enum

                    if (hasAlreadyBeenIntegrated(pathToAppImage)) {
                        if (desktopFileHasBeenUpdatedSinceLastUpdate(pathToAppImage)) {
                            throw WouldNotIntegrateError("AppImage has been integrated already and doesn't need to be re-integrated");
                        }

                        qout() << "AppImage has already been integrated, but needs to be reintegrated" << endl;
                    }


                    // check for X-AppImage-Integrate=false
                    auto shallNotBeIntegrated = appimage_shall_not_be_integrated(pathToAppImage.toStdString().c_str());
                    if (shallNotBeIntegrated < 0) {
                        throw CliError("AppImageLauncher error: appimage_shall_not_be_integrated() failed (returned " + QString::number(shallNotBeIntegrated) + ")");
                    } else if (shallNotBeIntegrated > 0) {
                        throw WouldNotIntegrateError("AppImage should not be integrated");
                    }

                    if (pathToAppImage.startsWith("/tmp/.mount_")) {
                        throw WouldNotIntegrateError("AppImages in AppImages are not supposed to be integrated");
                    }

                    // ignore terminal apps (fixes #2)
                    auto isTerminalApp = appimage_is_terminal_app(pathToAppImage.toStdString().c_str());
                    if (isTerminalApp < 0) {
                        throw CliError("AppImageLauncher error: appimage_is_terminal_app() failed (returned " + QString::number(isTerminalApp) + ")");
                    } else if (isTerminalApp > 0) {
                        throw WouldNotIntegrateError("Terminal AppImages should not be integrated");
                    }

                    qerr() << "AppImage should be integrated";
                }
            }
        }
    }
}
