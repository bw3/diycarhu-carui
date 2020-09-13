#include "amp.h"
#include "util.h"

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
#include <set>
#include <chrono>

Amp::Amp (int i2c_adptr, uint8_t i2c_addr) {
    std::vector<uint8_t> a = {i2c_addr};
    Amp(i2c_adptr, a);
}

Amp::Amp (int i2c_adptr, std::vector<uint8_t> i2c_addr) {
    m_cmd_mutex.lock();
    std::thread t(&Amp::run, this, i2c_adptr, i2c_addr);
    m_thread.swap(t);
}

void Amp::run(int i2c_adptr, std::vector<uint8_t> i2c_addr) {
    for(size_t i=0;i<i2c_addr.size();i++) {
        int _i2c_fd;
        char filename[20];
        snprintf(filename, 19, "/dev/i2c-%d", i2c_adptr);
        _i2c_fd = open(filename, O_RDWR);
        if (_i2c_fd < 0) {
            exit(1);
        }
        if (ioctl(_i2c_fd, I2C_SLAVE, i2c_addr[i]) < 0) {
            exit(1);
        }
        uint16_t bytesSent = 0;
        std::vector<uint8_t> writeData = {0x00, 0x00, 0x31, 0x52, 0x44, 0x55,0xCF, 0xCF, 0xCF, 0xCF, 0x02};
        write(_i2c_fd, writeData.data(), writeData.size());
        i2c_fd.push_back(_i2c_fd);
    }
    m_cmd_mutex.unlock();
}

void Amp::play() {
    std::lock_guard l(m_cmd_mutex);
    for(size_t i=0;i<i2c_fd.size();i++) {
        std::vector<uint8_t> writeData = {0x04, 0x00};
        write(i2c_fd[i], writeData.data(), writeData.size());
    }
}

void Amp::hi_z() {
    std::lock_guard l(m_cmd_mutex);
    for(size_t i=0;i<i2c_fd.size();i++) {
        std::vector<uint8_t> writeData = {0x04, 0x55};
        write(i2c_fd[i], writeData.data(), writeData.size());
    }
}

void Amp::mute() {
    std::lock_guard l(m_cmd_mutex);
    for(size_t i=0;i<i2c_fd.size();i++) {
        std::vector<uint8_t> writeData = {0x04, 0xCC};
        write(i2c_fd[i], writeData.data(), writeData.size());
    }
}

void Amp::line_outputs(QList<int> list) {
    std::lock_guard l(m_cmd_mutex);
    std::set<int> lineSet;
    for(int i=0;i<list.length();i++) {
        lineSet.insert(list[i]);
    }
    for(size_t i=0;i<i2c_fd.size();i++) {
        uint8_t modeControl = 0x00;
        if( lineSet.count(i*4) == 1 ) {
            modeControl |= 0x08;
        }
        if( lineSet.count(i*4 + 1) == 1) {
            modeControl |= 0x04;
        }
        if( lineSet.count(i*4 + 2) == 1) {
            modeControl |= 0x02;
        }
        if( lineSet.count(i*4 + 3) == 1) {
            modeControl |= 0x01;
        }
        std::vector<uint8_t> writeData = {0x00, modeControl};
        write(i2c_fd[i], writeData.data(), writeData.size());
    }

}
