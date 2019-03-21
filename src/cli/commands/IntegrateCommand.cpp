// library headers
#include <QFileInfo>
#include <QLoggingCategory>

// local headers
#include "IntegrateCommand.h"
#include "exceptions.h"
#include "shared.h"

namespace appimagelauncher {
    namespace cli {
        namespace commands {
            void IntegrateCommand::exec(QList<QString> arguments) {
                if (arguments.empty()) {
                    throw InvalidArgumentsError("No AppImages passed on commandline");
                }

                // make sure all AppImages exist on disk before further processing
                for (const auto& path : arguments) {
                    if (!QFileInfo(path).exists()) {
                        throw UsageError("could not find file " + path);
                    }
                }

                for (const auto& pathToAppImage : arguments) {
                    qInfo() << "Processing" << pathToAppImage.toStdString().c_str();

                    if (hasAlreadyBeenIntegrated(pathToAppImage)) {
                        if (desktopFileHasBeenUpdatedSinceLastUpdate(pathToAppImage)) {
                            qInfo() << "AppImage has been integrated already and doesn't need to be re-integrated, skipping";
                            continue;
                        }

                        qInfo() << "AppImage has already been integrated, but needs to be reintegrated";
                    }

                    auto pathToIntegratedAppImage = buildPathToIntegratedAppImage(pathToAppImage);

                    if (QFileInfo(pathToAppImage).absoluteFilePath() != QFileInfo(pathToIntegratedAppImage).absoluteFilePath()) {
                        qInfo() << "Moving AppImage to integration directory";

                        if (!QFile(pathToIntegratedAppImage).remove()) {
                            qCritical() << "Could not move AppImage into integration directory (error: failed to overwrite existing file)";
                            continue;
                        }

                        if (!QFile(pathToAppImage).rename(pathToIntegratedAppImage)) {
                            qCritical() << "Cannot move AppImage to integration directory (permission problem?), attempting to copy instead";

                            if (!QFile(pathToAppImage).copy(pathToIntegratedAppImage)) {
                                qCritical() << "Failed to copy AppImage, giving up";
                                continue;
                            }
                        }
                    } else {
                        qInfo() << "AppImage already in integration directory";
                    }

                    installDesktopFileAndIcons(pathToAppImage);
                }
            }
        }
    }
}
