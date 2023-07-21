#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "main.h"
#include "ublox_gps.h"
#include "dummy_gps.h"
#include "radio.h"
#include "amp.h"
#include "arduino.h"
#include "SigmaDSP/SigmaDSP.h"
#include "mpd.h"
#include "bluetooth.h"

#include <thread>

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <cmath>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>

std::string get_home_dir() {
    if (const char *xdg_conf = std::getenv("XDG_CONFIG_HOME")) {
        return std::string(xdg_conf) + "/carui/";
    }
    if (const char *home = std::getenv("HOME")) {
        return std::string(home) + "/.config/carui/";
    }
    return "";
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QQmlApplicationEngine engine;

#ifdef REAL_HARDWARE
    QApplication::setOverrideCursor(Qt::BlankCursor);
    UbloxGps gps;
    engine.rootContext()->setContextProperty("Gps", &gps);
    Amp amp(6, {0x6B,0x6A});
    engine.rootContext()->setContextProperty("Amp", &amp);
    SigmaDSP dsp(6, 0x34);
    engine.rootContext()->setContextProperty("Dsp", &dsp);
    dsp.loadXml(("/root/.config/carui/dsp.xml"));
    dsp.begin(6);
    Radio radio;
    engine.rootContext()->setContextProperty("Radio", &radio);
    Arduino a;
    engine.rootContext()->setContextProperty("Arduino", &a);
#else
    DummyGps gps;
    engine.rootContext()->setContextProperty("Gps", &gps);
#endif
    Mpd mpd;
    engine.rootContext()->setContextProperty("Mpd", &mpd);
    Bluetooth bluetooth;
    engine.rootContext()->setContextProperty("Bluetooth", &bluetooth);

    engine.load(QUrl(QStringLiteral("qrc://⁄src/main.qml")));
    return app.exec();
}
