import QtQuick 2.0

QtObject {
    property int volume: -20
    property string source_type
    property bool mute: false

    onMuteChanged: console.log("Mute = " +mute.toString())

    function init() {
        Amp.line_outputs([7]);
        Dsp.volume_slew("Master_Volume", volume);
        Dsp.EQparametric("Sub_EQ", 0, 0, false, 45, 1, 0);
        Dsp.EQhighpass("FT_EQ", 0, 0, false, 80, 0.7)
        Dsp.audioDelay("Sub_Delay", 0.1);
    }
    function incVolume() {
        volume+= 1;
        Dsp.volume_slew("Master_Volume", volume);
    }
    function decVolume() {
        volume-= 1;
        Dsp.volume_slew("Master_Volume", volume);
    }
    function nextTrack() {
        switch(source_type) {
           case "aux":
                break;
           case "usb":
                Mpd.next();
                break;
           case "fm":
                Radio.seekNext();
                break;
        }
    }
    function prevTrack() {
        switch(source_type) {
           case "aux":
                break;
           case "usb":
                Mpd.prev();
                break;
           case "fm":
                Radio.seekPrev();
                break;
        }
    }
    function toggle_mute() {
        if(mute) {
            Dsp.volume_slew("Master_Volume", volume);
            if(source_type == "usb") {
                Mpd.resume();
            }
        } else {
            Dsp.volume_slew("Master_Volume", -100);
            Mpd.pause();
        }
        mute = !mute;
    }
    function selectSource(type) {
        source_box.clear();
        source_type = type;
        switch(type) {
           case "aux":
                Dsp.mux("Source", 0)
                source_box.push("AuxSourceBox.qml");
                Mpd.pause();
                break;
           case "usb":
                Dsp.mux("Source", 1)
                source_box.push("MpdSourceBox.qml");
                Mpd.resume();
                break;
           case "fm":
                Dsp.mux("Source", 2)
                source_box.push("RadioSourceBox.qml");
                Mpd.pause();
                break;
        }
    }
}
