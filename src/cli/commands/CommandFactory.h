#pragma once

// system headers
#include <memory>

// library headers
#include <QString>

// local headers
#include <Command.h>

/**
 * Creates Commands.
 */

namespace appimagelauncher {
    namespace cli {
        namespace commands {
            class CommandFactory {
            public:
                static std::shared_ptr<Command> getCommandByName(const QString&);
            };
        }
    }
}
