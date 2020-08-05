#ifndef GPS_H
#define GPS_H

#include <QObject>
#include <QString>
#include <QGeoCoordinate>

#include <thread>

class Gps : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QGeoCoordinate position READ position NOTIFY positionChanged)
    Q_PROPERTY(double bearing READ bearing NOTIFY positionChanged)

public:
    explicit Gps (QObject *parent = nullptr);

    QGeoCoordinate position();
    double bearing();
    void setBearing(double b);
    void setPosition(double lat, double lon);

signals:
    void positionChanged();
protected:
    std::thread m_update_thread;
    QGeoCoordinate m_position;
    double m_bearing;
};

#endif // GPS_H

