#pragma once

// system headers
#include <stdexcept>

// library headers
#include <QString>

namespace appimagelauncher {
    namespace cli {
        namespace commands {
            class CliError : public std::runtime_error {
            public:
                explicit CliError(const QString& message) : std::runtime_error(message.toStdString()) {}
            };

            class CommandNotFoundError : public std::runtime_error {
            private:
                QString commandName;

            public:
                explicit CommandNotFoundError(const QString& commandName) : std::runtime_error(
                        "No such command available: " + commandName.toStdString()), commandName(commandName) {}

                QString getCommandName() const {
                    return commandName;
                }
            };

            class UsageError : public CliError {
            public:
                using CliError::CliError;
            };

            class InvalidArgumentsError : public UsageError {
            public:
                using UsageError::UsageError;
            };
        }
    }
}
