import QtQuick 2.0

Menu {
    id: menu
    model: ListModel {
        ListElement {
            name: "(back)"
            action: function action() {
                stack.pop()
            }
        }
        ListElement {
            name: "USB"
            type: "usb"
        }
        ListElement {
            name: "Aux"
            type: "aux"
        }
        ListElement {
            name: "FM Radio"
            type: "fm"
        }
        ListElement {
            name: "Bluetooth"
            type: "bluetooth"
        }
    }

    function action() {
        var element = menu.model.get(menu.currentIndex);
        audioControl.selectSource(element.type);
        stack.pop()
    }
}
