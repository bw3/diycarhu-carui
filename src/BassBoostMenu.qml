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
            name: "0db"
            action: function action() {
                audioControl.bass_boost(0);
                stack.pop()
            }
        }
        ListElement {
            name: "6db"
            action: function action() {
                audioControl.bass_boost(6);
                stack.pop()
            }
        }
        ListElement {
            name: "12db"
            action: function action() {
                audioControl.bass_boost(12);
                stack.pop()
            }
        }
        ListElement {
            name: "18db"
            action: function action() {
                audioControl.bass_boost(18);
                stack.pop()
            }
        }
    }
}
