// library headers
#include <QFileInfo>

// local headers
#include "IntegrateCommand.h"
#include "exceptions.h"
#include "shared.h"
#include "logging.h"

namespace appimagelauncher {
    namespace cli {
        namespace commands {
            void IntegrateCommand::exec(QList<QString> arguments) {
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
                    qout() << "Processing " << pathToAppImage << endl;

                    if (!QFileInfo(pathToAppImage).isFile()) {
                        qerr() << "Warning: Not a file, skipping: " << pathToAppImage << endl;
                        continue;
                    }

                    if (!isAppImage(pathToAppImage)) {
                        qerr() << "Warning: Not an AppImage, skipping: " << pathToAppImage << endl;
                        continue;
                    }

                    if (hasAlreadyBeenIntegrated(pathToAppImage)) {
                        if (desktopFileHasBeenUpdatedSinceLastUpdate(pathToAppImage)) {
                            qout() << "AppImage has been integrated already and doesn't need to be re-integrated, skipping" << endl;
                            continue;
                        }

                        qout() << "AppImage has already been integrated, but needs to be reintegrated" << endl;
                    }

                    auto pathToIntegratedAppImage = buildPathToIntegratedAppImage(pathToAppImage);

                    // make sure integration directory exists
                    // (important for new installations)
                    // pretty ugly, but well, one taketh what the Qt API giveth
                    QDir().mkdir(integratedAppImagesDestination().path());

                    // check if it's already in the right place
                    if (QFileInfo(pathToAppImage).absoluteFilePath() != QFileInfo(pathToIntegratedAppImage).absoluteFilePath()) {
                        qout() << "Moving AppImage to integration directory" << endl;

                        if (QFile::exists(pathToIntegratedAppImage) && !QFile(pathToIntegratedAppImage).remove()) {
                            qerr() << "Could not move AppImage into integration directory (error: failed to overwrite existing file)" << endl;
                            continue;
                        }

                        if (!QFile(pathToAppImage).rename(pathToIntegratedAppImage)) {
                            qerr() << "Cannot move AppImage to integration directory (permission problem?), attempting to copy instead" << endl;

                            if (!QFile(pathToAppImage).copy(pathToIntegratedAppImage)) {
                                throw CliError("Failed to copy AppImage, giving up");
                            }
                        }
                    } else {
                        qout() << "AppImage already in integration directory" << endl;
                    }

                    installDesktopFileAndIcons(pathToIntegratedAppImage);
                }
            }
        }
    }
}
