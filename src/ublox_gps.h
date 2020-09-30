#ifndef UBLOX_GPS_H
#define UBLOX_GPS_H

#include <QObject>
#include <QString>
#include <QGeoCoordinate>

#include <thread>

#include "gps.h"

class UbloxGps : public Gps
{
public:
    explicit UbloxGps (QObject *parent = nullptr);

    void runRecv();
    void runSetup();

signals:
    void positionChanged();
private:
    void decodeNMEA(char* str);
    void decodeHNR(char* str);

    int uart_fd;
    bool clock_set;
};

#endif // UBLOX_GPS_H

