import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    BusyIndicator {
        running: true
        width: 64
        height: 64
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
    }
}
