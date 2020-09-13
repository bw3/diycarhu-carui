import QtQuick 2.0

Item {
    Timer {
        interval: 500; running: true; repeat: true
        onTriggered: time.text = "                                " + (new Date()).toString()
    }
    Text {
        text: "Volume: "+ (audioControl.mute ? "MUTE" : audioControl.volume + " dB")
        color: "white"
        width: 100
    }
    Text {
        id: time
        color: "white"
    }
}
