// library includes
#include <QDBusConnection>
#include <QDebug>

// local includes
#include "shared.h"
#include "DBusServer.h"


DBusServer::DBusServer(QObject* parent) : QObject(parent) {
    auto bus = QDBusConnection::sessionBus();

    if (!bus.isConnected()) {
        throw DBusSessionBusNotAvailable(bus);
    }

    if (!bus.registerService(APPIMAGELAUNCHERD_DBUS_SERVICE_NAME)) {
        throw DBusServiceRegistrationFailed(bus);
    }

    if (!bus.registerObject("/", APPIMAGELAUNCHERD_DBUS_SERVICE_NAME, this, QDBusConnection::ExportAllSlots)) {
        throw DBusObjectRegistrationFailed(bus);
    }
}


bool DBusServer::updateDesktopDatabaseAndIconCaches()  {
    qDebug() << "[DBus] Call to updateDesktopDatabaseAndIconCaches received, processing";
    auto rv = ::updateDesktopDatabaseAndIconCaches();
    qDebug() << "[DBus] Call to updateDesktopDatabaseAndIconCaches done, rv:" << rv;
    return rv;
}
