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
            name: "Example"
            action: function action() {
                navRoute.setDestination(43.15623328742955, -77.6104715698674);
                stack.pop()
            }
        }
    }
}
