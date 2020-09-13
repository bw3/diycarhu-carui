import QtQuick 2.0

ListModel {
    ListElement {
        name: "Go back"
        action: function action() {
            stack.pop()
        }
    }
}
