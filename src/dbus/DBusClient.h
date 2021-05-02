#pragma once

// library includes
#include <QObject>
#include <QDBusInterface>

// local includes
#include "dbus_exceptions.h"


class DBusClient : public QObject {
    Q_OBJECT

private:
    QDBusInterface* interface;

public:
    typedef enum {BLOCKING, NON_BLOCKING} RpcMode;

    explicit DBusClient(QObject* parent);

    bool updateDesktopDatabaseAndIconCaches(RpcMode mode);

private:
    QVariantList makeRequest(const QString& method, const RpcMode mode, const QVariantList& requestArgs);
};
