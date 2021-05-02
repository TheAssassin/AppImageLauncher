// library includes
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>

// local includes
#include "DBusClient.h"
#include "dbus_exceptions.h"


DBusClient::DBusClient(QObject* parent) : QObject(parent) {
    auto bus = QDBusConnection::sessionBus();

    if (!bus.isConnected()) {
        throw DBusSessionBusNotAvailable(bus);
    }

    this->interface = new QDBusInterface(APPIMAGELAUNCHERD_DBUS_SERVICE_NAME, "/", APPIMAGELAUNCHERD_DBUS_SERVICE_NAME, bus, this);

    if (!this->interface->isValid()) {
        throw DBusInterfaceNotAvailable(bus);
    }
}


bool DBusClient::updateDesktopDatabaseAndIconCaches(RpcMode mode) {
    auto rv = DBusClient::makeRequest("updateDesktopDatabaseAndIconCaches", mode, QVariantList());

    if (mode == RpcMode::BLOCKING) {
        return rv.front().toBool();
    } else {
        return true;
    }
}


QVariantList DBusClient::makeRequest(const QString& method, const RpcMode mode, const QVariantList& requestArgs) {
    QDBus::CallMode callMode = QDBus::Block;

    if (mode == RpcMode::NON_BLOCKING) {
        callMode = QDBus::NoBlock;
    }

    qDebug() << "Request to" << method << "mode" << mode << "requestArgs" << requestArgs;

    const auto replyMessage = this->interface->callWithArgumentList(callMode, "updateDesktopDatabaseAndIconCaches", requestArgs);

    if (mode == RpcMode::BLOCKING) {
        qDebug() << "Blocking request, reply:" << replyMessage;

        if(replyMessage.type() != QDBusMessage::ReplyMessage) {
            throw DBusInvalidReplyReceived(this->interface->connection());
        }

        return replyMessage.arguments();
    } else {
        qDebug() << "Non-blocking request, not waiting for result, returning empty argument list";
        return QVariantList();
    }
}
