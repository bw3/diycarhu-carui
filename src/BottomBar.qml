import QtQuick 2.0

Item {
    Timer {
        interval: 500; running: true; repeat: true
        onTriggered: updateRight()
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

    function updateRight() {
        const date = new Date();
        let text = date.getHours() + ":" + date.getMinutes().toString().padStart(2,"0");
        if( navRoute.timeLegSec > 30 ) {
            const dateArrival = new Date(date.getTime() + navRoute.timeLegSec*1000);
            text += " (" +navRoute.distanceLegStr + " " + navRoute.timeLegStr + ") ";
            text += dateArrival.getHours() + ":" + dateArrival.getMinutes().toString().padStart(2,"0");
        }
        text += "  " + Math.round(Gps.temperature) + " C";
        onTriggered: time.text = "                                        " + text;
    }
}
