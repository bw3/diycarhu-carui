#include "gps.h"

Gps::Gps (QObject *parent ) { }

QGeoCoordinate Gps::position() {
    return m_position;
}

double Gps::bearing() {
    return m_bearing;
}

void Gps::setBearing(double b) {
    m_bearing = b;
    emit positionChanged();
}

void Gps::setPosition(double lat, double lon) {
    m_position.setLatitude(lat);
    m_position.setLongitude(lon);
    emit positionChanged();
}
