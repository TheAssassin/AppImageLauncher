// system headers
#include <sstream>

// library headers
#include <QCoreApplication>
#include <QCommandLineParser>

// local headers
#include "CommandFactory.h"
#include "exceptions.h"
#include "logging.h"

using namespace appimagelauncher::cli;
using namespace appimagelauncher::cli::commands;

int main(int argc, char** argv) {
    // we don't have support for UI (and don't want that), so let's tell shared to not display dialogs
    setenv("_FORCE_HEADLESS", "1", 1);

    QCoreApplication app(argc, argv);

    std::ostringstream version;
    version << "version " << APPIMAGELAUNCHER_VERSION << " "
            << "(git commit " << APPIMAGELAUNCHER_GIT_COMMIT << "), built on "
            << APPIMAGELAUNCHER_BUILD_DATE;
    QCoreApplication::setApplicationVersion(QString::fromStdString(version.str()));

    QCommandLineParser parser;

    parser.addPositionalArgument("<command>", "Command to run (see help for more information");
    parser.addPositionalArgument("[...]", "command-specific additional arguments");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);

    auto posArgs = parser.positionalArguments();

    if (posArgs.isEmpty()) {
        qerr() << parser.helpText().toStdString().c_str() << endl;

        qerr() << "Available commands:" << endl;
        qerr() << "  integrate    Integrate AppImages passed as commandline arguments" << endl;
        qerr() << "  unintegrate  Unintegrate AppImages passed as commandline arguments" << endl;

        return 2;
    }

    auto commandName = posArgs.front();
    posArgs.pop_front();

    try {
        auto command = CommandFactory::getCommandByName(commandName);
        command->exec(posArgs);
    } catch (const CommandNotFoundError& e) {
        qerr() << e.what() << endl;
        return 1;
    } catch (const InvalidArgumentsError& e) {
        qerr() << "Invalid arguments: " << e.what() << endl;
        return 3;
    } catch (const UsageError& e) {
        qerr() << "Usage error: " << e.what() << endl;
        return 3;
    }

    return 0;
}
