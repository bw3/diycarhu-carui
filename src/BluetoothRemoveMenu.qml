import QtQuick 2.0

Item {
    Text {
        color: "white"
        id: desc
        font.pointSize: 24
        text: "Remove Bluetooth Device"
        wrapMode: Text.Wrap
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
    Menu {
        id: bluetoothRemoveMenu
        anchors.top: desc.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        model: ListModel {
            id: bluetoothRemoveModel
            ListElement {
                name: "(back)"
                action: function action() {
                    stack.pop()
                }
            }
        }
        function action() {
            var element = bluetoothRemoveMenu.model.get(bluetoothRemoveMenu.currentIndex);
            console.log(element.mac_addr);
            Bluetooth.removeDevice(element.mac_addr);
            stack.pop()
        }

    }
    Component.onCompleted: {
        var devices = Bluetooth.getPairedDevices();
        for (var prop in devices) {
            console.log("Object item:", prop, "=", devices[prop])
            bluetoothRemoveModel.append({"name":prop,"mac_addr":devices[prop]});
        }
    }
    Component.onDestruction: {
    }
}
