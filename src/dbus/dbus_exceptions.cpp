// system includes
#include <sstream>

// local includes
#include "dbus_exceptions.h"


std::string DBusError::makeErrorMessage(const QDBusConnection& bus, const QString& what) {
    auto busErrorMessage = bus.lastError().message().toStdString();

    std::ostringstream ofs{};
    ofs << "DBus Error" << ": " << busErrorMessage;

    if (!what.isEmpty()) {
        ofs << " (" << what.toStdString() << ")";
    }

    return ofs.str();
}

DBusError::DBusError(const QDBusConnection& bus, const QString& what) : std::runtime_error(makeErrorMessage(bus, what)) {}

DBusSessionBusNotAvailable::DBusSessionBusNotAvailable(const QDBusConnection& bus) : DBusError(bus, "Could not connect to DBus session bus") {}

DBusInterfaceNotAvailable::DBusInterfaceNotAvailable(const QDBusConnection& bus) : DBusError(bus, "DBus interface not available") {}

DBusServiceRegistrationFailed::DBusServiceRegistrationFailed(const QDBusConnection& bus) : DBusError(bus, QString("Failed to register service ") + APPIMAGELAUNCHERD_DBUS_SERVICE_NAME) {}

DBusObjectRegistrationFailed::DBusObjectRegistrationFailed(const QDBusConnection& bus) : DBusError(bus, "Failed to register object /") {}

DBusInvalidReplyReceived::DBusInvalidReplyReceived(const QDBusConnection& bus) : DBusError(bus, "Invalid reply received from RPC") {}
