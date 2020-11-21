#ifndef DUMMY_GPS_H
#define DUMMY_GPS_H

#include <QObject>
#include <QString>
#include <QGeoCoordinate>

#include <thread>

#include "gps.h"

class DummyGps : public Gps
{
    Q_OBJECT
public:
    explicit DummyGps (QObject *parent = nullptr);

    Q_INVOKABLE void dummyMove(double meters);
    Q_INVOKABLE void dummyRotate(double degrees);

signals:
protected:
    std::thread m_update_thread;
};

#endif // DUMMY_GPS_H

