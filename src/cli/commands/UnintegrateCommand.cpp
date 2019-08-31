// library headers
#include <QFileInfo>

// local headers
#include "UnintegrateCommand.h"
#include "exceptions.h"
#include "shared.h"
#include "logging.h"

namespace appimagelauncher {
    namespace cli {
        namespace commands {
            void UnintegrateCommand::exec(QList<QString> arguments) {
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

                    if (!hasAlreadyBeenIntegrated(pathToAppImage)) {
                        qout() << "AppImage has not been integrated yet, skipping" << endl;
                        continue;
                    }

                    unregisterAppImage(pathToAppImage);
                }
            }
        }
    }
}
