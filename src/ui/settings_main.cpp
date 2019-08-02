// libraries
#include <QIcon>
#include <QDebug>
#include <QApplication>

// local
#include "settings_dialog.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    // The theme icons used by the application may not be available on all target systems therefore a fallback theme
    // must be used to ensure that the icons will be always displayed
#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    QIcon::setFallbackThemeName("fallback");
#else
    // QIcon::setFallbackThemeName is not available until Qt 5.12
    QIcon::setThemeName("fallback");
#endif

    SettingsDialog dialog;
    dialog.show();

    return app.exec();
}
