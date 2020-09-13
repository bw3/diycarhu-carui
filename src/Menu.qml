import QtQuick 2

ListView {
    id: listView
    function action() {
        stack.pop()
    }
    focus: true
    highlight: Rectangle { color: "brown"; radius: 2 }
    clip: true
    model: ListModel {
        ListElement {
            name: "undefined menu"
        }
    }
    delegate: Text {
        text: name
        color: "white"
        font.pointSize: 20
        height: 40
        wrapMode: Text.WrapAnywhere
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if( typeof model.action === "function" ) {
                    model.action();
                } else {
                    listView.currentIndex = index;
                    listView.action();
                }
            }
        }
    }
}
