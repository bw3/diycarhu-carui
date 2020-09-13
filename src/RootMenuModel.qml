import QtQuick 2.0

ListModel {
    ListElement {
        name: "Select Source"
        action: function action() {
            stack.push('SourceMenu.qml');
        }
    }
    ListElement {
        name: "Pair Bluetooth"
        action: function action() {
            stack.push('BluetoothPairMenu.qml');
        }
    }
    ListElement {
        name: "Test Menu"
        action: function action() {
            stack.push('Menu.qml');
        }
    }
    ListElement {
        name: "Test Menu"
        action: function action() {
            stack.push('Menu.qml');
        }
    }
    ListElement {
        name: "Test Menu"
        action: function action() {
            stack.push('Menu.qml');
        }
    }
    ListElement {
        name: "Test Menu"
        action: function action() {
            stack.push('Menu.qml');
        }
    }
    ListElement {
        name: "Test Menu"
        action: function action() {
            stack.push('Menu.qml');
        }
    }
    ListElement {
        name: "Test Menu7"
        action: function action() {
            stack.push('Menu.qml');
        }
    }

}
