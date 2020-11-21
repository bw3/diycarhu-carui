import QtQuick 2.0

Item {
    property string icon: "ferry"
    property string line1: "Line 1"
    property string line2: "Line 2"
    property string exit: ""
    property string distance: "? mi"

    Rectangle {
        color: "green"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
    }

    Image {
        id: "nav_icon"
        source: "nav_icons/" + icon +"-white.svg"
        anchors.left: parent.left
        anchors.top: parent.top
        height: parent.height - nav_dist.height
        width: parent.height - nav_dist.height
        fillMode: Image.PreserveAspectFit
        sourceSize.width: 200
        sourceSize.height: 200
    }
    
    Text {
        id: "nav_dist"
        color: "white"
        font.pointSize: 10
        text: distance
        height: 16
        width: nav_icon.width
        anchors.left: parent.left
        anchors.top: nav_icon.bottom
        anchors.bottom: parent.bottom
        anchors.right: line2.left
        horizontalAlignment: Text.AlignHCenter
    }

    Text {
        id: "nav_exit"
        color: "white"
        font.pointSize: 24
        text: exit
        anchors.left: nav_icon.right
        anchors.top: parent.top
    }

    Rectangle {
        border.color: "white"
        color: "transparent"
        anchors.left: nav_exit.left
        anchors.right: nav_exit.right
        anchors.top: nav_exit.top
        anchors.bottom: nav_exit.bottom
    }

    Text {
        id: "nav_text1"
        color: "white"
        font.pointSize: 24
        text: line1
        anchors.left: nav_exit.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 5
    }

    Text {
        id: "nav_text2"
        color: "white"
        font.pointSize: 24
        text: line2
        anchors.left: nav_exit.right
        anchors.right: parent.right
        anchors.top: nav_text1.bottom
        anchors.bottom: parent.bottom
        anchors.leftMargin: 5
    }

}
