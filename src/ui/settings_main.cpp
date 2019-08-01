// libraries
#include <QIcon>
#include <QDebug>
#include <QApplication>

// local
#include "settings_dialog.h"

int main(int argc, char** argv) {
    // QIcon::setFallbackThemeName doesn't have effect on gtk styles 
    QIcon::setThemeName("fallback");

    QApplication app(argc, argv);
    SettingsDialog dialog;
    dialog.show();

    return app.exec();
}
