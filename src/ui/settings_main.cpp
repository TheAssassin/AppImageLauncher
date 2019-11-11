// libraries
#include <QApplication>

// local
#include <translationmanager.h>
#include "settings_dialog.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    TranslationManager mgr(app);

    SettingsDialog dialog;
    dialog.show();

    return app.exec();
}
