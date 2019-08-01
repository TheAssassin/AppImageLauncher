// libraries
#include <QIcon>
#include <QDebug>
#include <QApplication>

// local
#include "settings_dialog.h"

int main(int argc, char** argv) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    QIcon::setFallbackThemeName("fallback");
#else
    // QIcon::setFallbackThemeName is not available until Qt 5.12
    QIcon::setThemeName("fallback");
#endif

    QApplication app(argc, argv);
    SettingsDialog dialog;
    dialog.show();

    return app.exec();
}
