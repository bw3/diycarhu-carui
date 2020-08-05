#ifndef DUMMY_GPS_H
#define DUMMY_GPS_H

#include <QObject>
#include <QString>
#include <QGeoCoordinate>

#include <thread>

#include "gps.h"

class DummyGps : public Gps
{
public:
    explicit DummyGps (QObject *parent = nullptr);

    QGeoCoordinate position();
    double bearing();
    void setBearing(double b);

signals:
    void positionChanged();
protected:
    std::thread m_update_thread;
};

#endif // DUMMY_GPS_H

