// libraries
#include <QApplication>

// local
#include <translationmanager.h>
#include "settings_dialog.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationDisplayName("AppImageLauncher Settings");
    QApplication::setWindowIcon(QIcon(":/AppImageLauncher.svg"));

    TranslationManager mgr(app);

    SettingsDialog dialog;
    dialog.show();

    return app.exec();
}
