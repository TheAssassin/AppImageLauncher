#pragma once

// system includes
#include <stdexcept>

// library includes
#include <QDBusConnection>
#include <QDBusError>


class DBusError : public std::runtime_error {
private:
    static std::string makeErrorMessage(const QDBusConnection& bus, const QString& what);

public:
    explicit DBusError(const QDBusConnection& bus, const QString& what = "");
};


class DBusSessionBusNotAvailable : public DBusError {
public:
    explicit DBusSessionBusNotAvailable(const QDBusConnection& bus);
};


class DBusInterfaceNotAvailable : public DBusError {
public:
    explicit DBusInterfaceNotAvailable(const QDBusConnection& bus);
};


class DBusServiceRegistrationFailed : public DBusError {
public:
    explicit DBusServiceRegistrationFailed(const QDBusConnection& bus);
};


class DBusObjectRegistrationFailed : public DBusError {
public:
    explicit DBusObjectRegistrationFailed(const QDBusConnection& bus);
};


class DBusInvalidReplyReceived : public DBusError {
public:
    explicit DBusInvalidReplyReceived(const QDBusConnection& bus);
};
