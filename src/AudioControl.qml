import QtQuick 2.0

QtObject {
    property int volume: -30
    property string source_type
    property bool power: false
    property bool mute: false
    onMuteChanged: calc_mute()
    onPowerChanged: calc_mute()

    function init() {
        Amp.line_outputs([7]);
        //Amp.disable_outputs([0,1,2,3,4,5]);
        speakers6();
        Dsp.volume_slew("Master_Volume", volume);
        Dsp.EQhighpass("Center_EQ", 0, -20, false, 600, 0.7);
        Dsp.EQhighpass("FT_EQ", 0, -20, false, 600, 0.7);

        Dsp.EQlowpass("FW_EQ", 0, 0, false, 600, 0.7);
//        Dsp.EQhighpass("FW_EQ", 1, 0, false, 80, 0.7);
//        Dsp.EQlowshelf("FW_EQ", 3, 0, false, 110,1,-25);

        //Dsp.EQlowpass("RW_EQ", 0, 0, false, 4000, 0.7);

        Dsp.EQlowpass("Sub_EQ", 0, 0, false, 80, 0.7);
//        Dsp.EQparametric("Sub_EQ", 1, 0, false, 43.4, 2.011, -10.4);
//        Dsp.EQparametric("Sub_EQ", 2, 0, false, 66.9, 2, 9);
//        Dsp.EQparametric("Sub_EQ", 3, 0, false, 70.5, 2.187, -5.4);
        Dsp.EQlowshelf("Sub_EQ", 3, 0, false, 60,1,0);
        
        //Dsp.EQparametric("Sub_EQ", 4, 0, false, 59, 7.709, -4.9);
        //Dsp.EQparametric("Sub_EQ", 5 0, false, 54.6, 18.827, -5.8);

       //Dsp.audioDelay("Sub_Delay", 0.1);
    }

    function bass_boost(boost) {
        Dsp.EQlowshelf("Sub_EQ", 3, 0, false, 60,1,boost);
    }

    function speakers5() {
        Dsp.mux("Center_Select",2);
        Amp.disable_outputs([2,3,5]);
        calc_mute();
    }
    function speakers6() {
        Dsp.mux("Center_Select",1);
        Amp.disable_outputs([2,3]);
        calc_mute();
    }
    function speakers8() {
        Dsp.mux("Center_Select",1);
        Amp.disable_outputs([]);
        calc_mute();
    }

    function incVolume() {
        if(!mute) {
            volume+= 1;
        }
        Dsp.volume_slew("Master_Volume", volume);
    }
    function decVolume() {
        if(!mute || volume > -30) {
            volume-= 1;
        }
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
        mute = !mute;
        calc_mute();
    }
    function calc_mute() {
        if(power) {
            Amp.play();
            Arduino.sendCmd("A1");
        } else {
            Amp.hi_z();
            Arduino.sendCmd("A0");
        }
        if(mute || !power) {
            Dsp.volume_slew("Master_Volume", -100);
            Mpd.pause();
        }
        if(!mute && power) {
            Dsp.volume_slew("Master_Volume", volume);
            if(source_type == "usb") {
                Mpd.resume();
            }
        }
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
