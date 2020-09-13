import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Controls 2.0
import QtLocation 5.6
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

Rectangle {
    id: nav_box
    height: 70
    color: "green"
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
    center: Gps.position.atDistanceAndAzimuth(500,Gps.bearing) //ps.position //QtPositioning.coordinate(43.162, -77.574) // Oslo
    tilt: 45
    bearing: Gps.bearing
    zoomLevel: 14
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
            case "L0":
                Arduino.sendCmd("P100");
                break;
            case "L1":
                Arduino.sendCmd("P250");
                break;
            case "V1":
                Amp.play();
                Arduino.sendCmd("A1");
                break;
            case "V0":
                Amp.hi_z();
                Arduino.sendCmd("A0");
                break;
        }
    }
}

AudioControl {
    id: audioControl
}

}
