#ifndef UBLOX_GPS_H
#define UBLOX_GPS_H

#include <QObject>
#include <QString>
#include <QGeoCoordinate>

#include <thread>

#include "gps.h"

class UbloxGps : public Gps
{
    Q_OBJECT
    Q_PROPERTY(int fix_type READ fix_type NOTIFY hnr_signal)
    Q_PROPERTY(QString fix_type_desc READ fix_type_desc NOTIFY hnr_signal)
public:
    explicit UbloxGps (QObject *parent = nullptr);

    void runRecv();
    void runSetup();
    int fix_type() { return m_fix_type; }
    QString fix_type_desc();
signals:
    void positionChanged();
    void hnr_signal();
private:
    void decodeNMEA(char* str);
    void decodeHNR(char* str);

    int uart_fd;
    bool clock_set;

    int m_fix_type;
};

#endif // UBLOX_GPS_H

