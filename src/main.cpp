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
/*
printf("loading amp ... \n");
  int _i2c_fd;
  char filename[20];
  snprintf(filename, 19, "/dev/i2c-%d", 6);
  _i2c_fd = open(filename, O_RDWR);
  if (_i2c_fd < 0) {
    exit(1);
  }
  if (ioctl(_i2c_fd, I2C_SLAVE, 0x6B) < 0) {
    exit(1);
  }
  uint16_t bytesSent = 0;
  std::vector<uint8_t> writeData = {0x00, 0x00, 0x31, 0x62, 0x44, 0x00,0xA0, 0xA0, 0xA0, 0xA0};
    write(_i2c_fd, writeData.data(), writeData.size());

    SigmaDSP myDSP(0x34,0,0);
    printf("Loading dsp ...\n");
    myDSP.loadXml((get_home_dir() + "dsp.xml").c_str());
    myDSP.begin(6);*/
    //Radio radio;
    //    loadProgram(myDSP);
//  myDSP.writeRegister(CORE_REGISTER_R0_ADDR, CORE_REGISTER_R0_SIZE, DSP_core_register_R0_data);
//  myDSP.writeRegisterBlock(PROGRAM_ADDR, PROGRAM_SIZE, DSP_program_data, PROGRAM_REGSIZE);
//  myDSP.writeRegisterBlock(PARAMETER_ADDR, PARAMETER_SIZE, DSP_parameter_data, PARAMETER_REGSIZE);
//  myDSP.writeRegister(HARDWARE_CONF_ADDR, HARDWARE_CONF_SIZE, DSP_hardware_conf_data);
//  myDSP.writeRegister(CORE_REGISTER_R4_ADDR, CORE_REGISTER_R4_SIZE, DSP_core_register_R4_data);
//  for(int8_t i = 0; i > -51; i--)
//  {
//    myDSP.volume_slew("Master Volume", i, 12); // (First volume slider address, dB value, optional slew value)
//    std::this_thread::sleep_for(std::chrono::milliseconds(200));
//  }
//
//  for(int8_t i = -50; i < 1; i++)
//  {
//    myDSP.volume_slew("Master Volume", i); // Slew rate is default 12
//    std::this_thread::sleep_for(std::chrono::milliseconds(200));
//  }

#ifdef REAL_HARDWARE
    UbloxGps gps;
    Amp amp;
    SigmaDSP myDSP(0x34,0,0);
    myDSP.loadXml(("/root/.config/carui/dsp.xml"));
    myDSP.begin(6);
//    Radio radio;
    Arduino a;
#else
    DummyGps gps;
#endif

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("Gps", &gps);
    engine.load(QUrl(QStringLiteral("qrc://⁄src/main.qml")));
    return app.exec();
}
