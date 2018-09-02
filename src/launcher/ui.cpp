#include <sstream>
#include <QtWidgets/QMessageBox>
#include "ui.h"

UI::UI() {

}

void UI::askIfAppImageFileShouldBeOverridden() {
    std::ostringstream message;
    message << QObject::tr("AppImage with same filename has already been integrated.").toStdString() << std::endl
            << std::endl
            << QObject::tr("Do you wish to overwrite the existing AppImage?").toStdString() << std::endl
            << QObject::tr(
                    "Choosing No will run the AppImage once, and leave the system in its current state.").toStdString();

    auto rv = QMessageBox::warning(
            nullptr,
            QObject::tr("Warning"),
            QString::fromStdString(message.str()),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes
    );

    if (rv == QMessageBox::No)
            emit dontOverrideAppImageFile();
    else
            emit overrideAppImageFile();
}

void UI::alertIntegrationFailed(const IntegrationFailed &ex) {
    QMessageBox::critical(
            nullptr,
            QObject::tr("Error"),
            ex.what()
    );
    emit alertIntegrationFailedCompleted();
}
