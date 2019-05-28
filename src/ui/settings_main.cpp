// libraries
#include <QApplication>

// local
#include "settings_dialog.h"
#include "shared.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    SettingsDialog dialog;
    dialog.setSettingsFile(getConfig());

    dialog.show();
    return app.exec();
}
