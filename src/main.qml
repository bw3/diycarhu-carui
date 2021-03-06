import QtQuick 2.6
import QtQuick.Window 2.0
import QtQuick.Controls 2.0
import QtLocation 5.12
import QtPositioning 5.6

import "startup.js" as Startup
 
Window {
width: 640
height: 400
visible: true
Component.onCompleted: {
    audioControl.init();
    audioControl.selectSource("usb");
    Arduino.sendCmd("S");
}
 
Plugin {
id: mapboxglPlugin
name: "mapboxgl"
PluginParameter { name: "mapboxgl.mapping.additional_style_urls"; value: "http://localhost:8553/v1/mbgl/style?style=mc-car-en" }
PluginParameter { name: "mapboxgl.mapping.cache.size"; value: "0" }
}

Rectangle {
    color: "black"
    anchors.fill: parent
}

//Rectangle {
//    id: nav_box
//    height: 70
//    color: "green"
//    anchors.left: parent.left
//    anchors.right: parent.horizontalCenter
//}

NavBox {
    id: nav_box
    height: 70
    anchors.left: parent.left
    anchors.right: parent.horizontalCenter
}


StackView {
    id: source_box
    height: 70
//    color: "white"
//    font.pointSize: 28
//    text: Mpd.title + "<br/>" + Mpd.artist
    anchors.left: parent.horizontalCenter
    anchors.right: parent.right
}

BottomBar {
    id: bottom_bar
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    height: 35
//    color: "red"
}
Map {
    id: map
    anchors.left: parent.left
    anchors.right: parent.horizontalCenter
    anchors.top: nav_box.bottom
    anchors.bottom: bottom_bar.top
    plugin: mapboxglPlugin
    center: Gps.position.atDistanceAndAzimuth(100,Gps.bearing) 
    tilt: 45
    bearing: Gps.bearing
    zoomLevel: 17
    MapPolyline {
        line.width: 3
        line.color: 'white'
        path: [
            Gps.position.atDistanceAndAzimuth(25,Gps.bearing-145),
            Gps.position,
            Gps.position.atDistanceAndAzimuth(25,Gps.bearing+145) 
        ]
    }
    MapPolyline {
        id: debugPolyline
        line.color: 'purple'
        line.width: 10
        path: []
    }
}

NavRoute {
    id: navRoute
}

Rectangle {
    id: i_item
    color: "yellow"
}

StackView {
    id: stack
    initialItem: Menu {
        model: RootMenuModel {

        }
    }
    anchors.left: parent.horizontalCenter
    anchors.right: parent.right
    anchors.top: nav_box.bottom
    anchors.bottom: bottom_bar.top
    popEnter: Transition {}
    popExit: Transition {}
    pushEnter: Transition {}
    pushExit: Transition {}
    replaceEnter: Transition {}
    replaceExit: Transition {}
}

Timer {
    property int count
    id: incVolumeTimer
    interval: 150
    repeat: true
    triggeredOnStart: true
    onTriggered: {
        if( count < 20) {
            audioControl.incVolume();
            count+=1;
        } else {
            running = false
        }
    }
}

Timer {
    property int count
    id: decVolumeTimer
    interval: 150
    repeat: true
    triggeredOnStart: true
    onTriggered: {
        if( count < 40) {
            audioControl.decVolume();
            count+=1;
        } else {
            running = false
        }
    }
}

Connections {
    target: Arduino
    onMsg: { 
        function findMenu() {
            if( stack.currentItem instanceof Menu ) {
                return stack.currentItem;
            } else {
                for(var i=0;i<stack.currentItem.children.length;i++) {
                    if( stack.currentItem.children[i] instanceof Menu ) {
                        return stack.currentItem.children[i];
                    }
                }
            }
        }
        switch(event) {
            case "E1C":
                var menu = findMenu();
                menu.incrementCurrentIndex();
                menu.positionViewAtIndex(stack.currentItem.currentIndex, ListView.Contain);
                break;
            case "E1A":
                var menu = findMenu();
                menu.decrementCurrentIndex();
                menu.positionViewAtIndex(stack.currentItem.currentIndex, ListView.Contain);
                break;
            case "B9U":
                var menu = findMenu();
                if( typeof menu.model.get(menu.currentIndex).action === "function" ) {
                    menu.model.get(menu.currentIndex).action();
                } else {
                    menu.action();
                }
                break;
            case "E0C":
                audioControl.incVolume();
                break;
            case "E0A":
                audioControl.decVolume();
                break;
            case "B8U":
                audioControl.toggle_mute();
                break;
            case "B0D":
                incVolumeTimer.count = 0
                incVolumeTimer.start()
                break;
            case "B0U":
                incVolumeTimer.stop()
                break;
            case "B1D":
                decVolumeTimer.count = 0
                decVolumeTimer.start()
                break;
            case "B1U":
                decVolumeTimer.stop();
                break;
            case "B2U":
                audioControl.nextTrack();
                break;
            case "B3U":
                audioControl.prevTrack();
                break;
            case "V1":
                audioControl.power = true;
                break;
            case "V0":
                audioControl.power = false;
                Arduino.sendCmd("P0");
                break;
            default:
                if(event.startsWith("L")) {
                    var brightness;
                    if(event.length == 1) {
                        brightness = 255;
                    } else {
                        var val = parseInt(event.substr(1));
                        if(val > 98) {
                            val = 255;
                        } else {
                            val = Math.round((val-7)*0.6+3);
                        }
                        console.log(val+"\n");
                        brightness = val;
                    }
                    if(audioControl.power) {
                        Arduino.sendCmd("P" + brightness.toString());
                    }
               }
        }
    }
}

AudioControl {
    id: audioControl
}

Shortcut {
    sequence: "F1"
    onActivated: Gps.dummyRotate(-5)
}

Shortcut {
    sequence: "F2"
    onActivated: Gps.dummyMove(10)
}

Shortcut {
    sequence: "F3"
    onActivated: Gps.dummyRotate(5)
}

Shortcut {
    sequence: "F4"
    onActivated: Gps.dummyMove(-5)
}

}
