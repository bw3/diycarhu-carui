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
        name: "PlayAllRandom"
        action: function action() {
            Mpd.play_all_random();
        }
    }
    ListElement {
        name: "Speakers5"
        action: function action() {
            audioControl.speakers5();
        }
    }
    ListElement {
        name: "Speakers6"
        action: function action() {
            audioControl.speakers6();
        }
    }
    ListElement {
        name: "Speakers8"
        action: function action() {
            audioControl.speakers8();
        }
    }
    ListElement {
        name: "Bass Boost"
        action: function action() {
            stack.push('BassBoostMenu.qml');
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
