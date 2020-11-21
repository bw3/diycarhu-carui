#include "dummy_gps.h"

void updateThread(Gps* gps) {
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
//        gps->setBearing(gps->bearing() - 1);
    }
}

DummyGps::DummyGps (QObject *parent ) : Gps(parent), m_update_thread(updateThread, this) {
    m_position.setLatitude(43.162);
    m_position.setLongitude(-77.574);
    m_bearing = 180;
}

void DummyGps::dummyMove(double meters) {
    m_position = m_position.atDistanceAndAzimuth(meters,m_bearing);
    emit positionChanged();

}

void DummyGps::dummyRotate(double degrees) {
    m_bearing += degrees;
    if(m_bearing < 0) {
        m_bearing += 360;
    }
    if(m_bearing > 360) {
        m_bearing -=360;
    }
    emit positionChanged();
}
