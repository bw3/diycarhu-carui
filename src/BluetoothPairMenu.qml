import QtQuick 2.0

Item {
    Text {
        color: "white"
        id: desc
        font.pointSize: 24
        text: "Please use your device to pair"
        wrapMode: Text.Wrap
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
    Menu {
        anchors.top: desc.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        model: ListModel {
            ListElement {
                name: "(back)"
                action: function action() {
                    stack.pop()
                }
            }
        }
    }
    Component.onCompleted: {
        Bluetooth.allowPairing();
        bluetoothPairingExitTimer.start();
    }
    Component.onDestruction: {
        Bluetooth.disallowPairing();
    }
    Timer {
    id: bluetoothPairingExitTimer
    interval: 120*1000
    repeat: false
    triggeredOnStart: false
    onTriggered: {
        stack.pop()
    }
}

}
