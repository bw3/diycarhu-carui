import QtQuick 2.0

Item {
    Timer {
        interval: 500; running: true; repeat: true
        onTriggered: time.text = "                                        " + (new Date()).getHours() + ":" + (new Date()).getMinutes().toString().padStart(2,"0")
    }
    Text {
        text: "Volume: "+ (audioControl.mute ? "MUTE" : audioControl.volume + " dB") + "  " + Gps.fix_type_desc
        color: "white"
        width: 10
        font.pointSize: 16
    }
    Text {
        id: time
        font.pointSize: 16
        color: "white"
    }
}
