// libraries
#include <QApplication>
#include <QtWidgets/QStyleFactory>

// local
#include "settings_dialog.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Oxygen"));
    SettingsDialog dialog;
    dialog.show();

    return app.exec();
}
