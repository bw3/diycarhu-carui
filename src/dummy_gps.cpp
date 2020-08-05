#include "dummy_gps.h"

void updateThread(Gps* gps) {
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        gps->setBearing(gps->bearing() - 1);
    }
}

DummyGps::DummyGps (QObject *parent ) : Gps(parent), m_update_thread(updateThread, this) {
    m_position.setLatitude(43.162);
    m_position.setLongitude(-77.574);
    m_bearing = 180;
}
