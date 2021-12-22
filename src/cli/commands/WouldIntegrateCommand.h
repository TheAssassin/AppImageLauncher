#pragma once

// local headers
#include "Command.h"

namespace appimagelauncher {
    namespace cli {
        namespace commands {
            /**
             * Check whether an AppImage would be integrated.
             */
            class WouldIntegrateCommand : public Command {
                void exec(QList<QString> arguments) final;
            };
        }
    }
}
