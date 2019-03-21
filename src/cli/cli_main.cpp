// library headers
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QLoggingCategory>

// local headers
#include "CommandFactory.h"
#include "exceptions.h"

using namespace appimagelauncher::cli;
using namespace appimagelauncher::cli::commands;

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;

    parser.addPositionalArgument("<command>", "Command to run (see help for more information");
    parser.addPositionalArgument("[...]", "command-specific additional arguments");
    parser.addHelpOption();

    parser.process(app);

    auto posArgs = parser.positionalArguments();

    if (posArgs.isEmpty()) {
        qCritical() << parser.helpText().toStdString().c_str();

        qCritical() << "Available commands:";
        qCritical() << "  integrate   Integrate AppImages passed as commandline arguments";

        return 2;
    }

    auto commandName = posArgs.front();
    posArgs.pop_front();

    try {
        auto command = CommandFactory::getCommandByName(commandName);
        command->exec(posArgs);
    } catch (const CommandNotFoundError& e) {
        qCritical() << e.what();
        return 1;
    } catch (const InvalidArgumentsError& e) {
        qCritical() << "Invalid arguments: " << e.what();
        return 3;
    } catch (const UsageError& e) {
        qCritical() << "Usage error: " << e.what();
        return 3;
    }

    return 0;
}
