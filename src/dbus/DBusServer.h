#pragma once

// library includes
#include <QObject>

// local includes
#include "dbus_exceptions.h"


class DBusServer : public QObject {
    Q_OBJECT
    Q_CLASSINFO("DBus interface", APPIMAGELAUNCHERD_DBUS_SERVICE_NAME)

public:
    explicit DBusServer(QObject* parent = nullptr);

public slots:
    static bool updateDesktopDatabaseAndIconCaches();
};
