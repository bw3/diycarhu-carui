#include "radio.h"
#include <linux/i2c-dev.h>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <chrono>
const uint8_t PWR_UP[] = {0x01, 0x00, 0xB0};
const uint8_t PWR_DOWN[] = {0x11};
const uint8_t REFCLK_FREQ[] = {0x12, 0x00, 0x02, 0x01, 0x7D, 0x00};
const uint8_t REFCLK_PRESCALE[] = {0x12, 0x00, 0x02, 0x02, 0x00, 0x60};
const uint8_t DO_FORMAT[] = {0x12, 0x00, 0x01, 0x02, 0x00, 0x80};
const uint8_t DO_SAMPLE_RATE[] = {0x12, 0x00, 0x01, 0x04, 0xBB, 0x80};
const uint8_t RDS_CONFIG[] = {0x12, 0x00, 0x15, 0x02, 0xAA, 0x01};
const uint8_t TUNE_START[] = {0x20, 0x00, 0x22, 0x2E, 0x00};
const uint8_t SEEK_NOWRAP[] = {0x21, 0x08};
const uint8_t _SEEK_NEXT[] = {0x21, 0x0C};
const uint8_t _SEEK_PREV[] = {0x21, 0x04};
const uint8_t GET_INT_STATUS[] = {0x14};
const uint8_t FM_TUNE_STATUS[] = {0x22, 0x00};
const uint8_t CLEAR_RDS[] = {0x24,0x02};
const uint8_t GET_RDS[] = {0x24,0x00};
const int CTS = 7;
const int RDSINT = 2;
const int STCINT = 0;

void Radio::wait_for_bit(int bit) {
    using namespace std::chrono;
    std::this_thread::sleep_for(300us);
    for(int i=0;i<100;i++) {
        if(bit <=6 ) {
            wait_for_bit(CTS);
            write(i2c_fd, GET_INT_STATUS, 1);
        }
        uint8_t status;
        read(i2c_fd, &status, 1);
        if( (status & 0x40) == 0x40 ) {
            printf("Error\n");
            return;
        }
        if( ((status >> bit) & 0x01) == 0x01 ) {
            return;
        }
//        printf("Waiting %d %#02x...\n",i,status);
        std::this_thread::sleep_for(10ms);
    }
}

Radio::Radio (QObject *parent ) {
    m_cmd = NONE;
    std::thread t(&Radio::run, this);
    m_thread.swap(t);
}

void Radio::run() {
    char filename[20];

    snprintf(filename, 19, "/dev/i2c-%d", 6);
    i2c_fd = open(filename, O_RDWR);
    if (i2c_fd < 0) {
        exit(1);
    }
    if (ioctl(i2c_fd, I2C_SLAVE, 0x11) < 0) {
        exit(1);
    }
    printf("Radio Power Up\n");
    write(i2c_fd, PWR_UP, 3);
    wait_for_bit(CTS);
    printf("Radio clk\n");
    write(i2c_fd, REFCLK_FREQ, 6);
    wait_for_bit(CTS);
    write(i2c_fd, REFCLK_PRESCALE, 6);
    wait_for_bit(CTS);
    {
        using namespace std::chrono;
        std::this_thread::sleep_for(500ms);
    }
    printf("sample rate\n");
    write(i2c_fd, DO_SAMPLE_RATE, 6);
    wait_for_bit(CTS);
    write(i2c_fd, RDS_CONFIG, 6);
    wait_for_bit(CTS);
    printf("Tune Start\n");
    write(i2c_fd, TUNE_START, 5);
    wait_for_bit(STCINT);
    bool wait = false;
    while(true) {
        if(wait) {
            std::unique_lock<std::mutex> l(m_cmd_mutex);
            if(m_cmd == NONE) {
                using namespace std::chrono;
                m_cmd_condition.wait_for(l, 1s);
            }
        }
        if(do_cmd()) {
            decode_fm_tune_status();
        } else {
            wait = (decode_rds() < 1);
        }
    }
}

bool Radio::do_cmd() {
    std::lock_guard l(m_cmd_mutex);
    switch(m_cmd) {
        case SEEK_NEXT:
            write(i2c_fd, _SEEK_NEXT, 2);
            wait_for_bit(STCINT);
            memset(m_radio_text,0,64);
            memset(m_radio_text_inactive,0,64);
            break;
        case SEEK_PREV:
            write(i2c_fd, _SEEK_PREV, 2);
            wait_for_bit(STCINT);
            memset(m_radio_text,0,64);
            memset(m_radio_text_inactive,0,64);
            break;
        case NONE:
            return false;
    }
    m_cmd = NONE;
    return true;
}

void Radio::seekNext() {
    std::lock_guard l(m_cmd_mutex);
    m_cmd = SEEK_NEXT;
    m_cmd_condition.notify_one();
}

void Radio::seekPrev() {
    std::lock_guard l(m_cmd_mutex);
    m_cmd = SEEK_PREV;
    m_cmd_condition.notify_one();
}

bool Radio::decode_fm_tune_status() {
    write(i2c_fd, FM_TUNE_STATUS, 2);
    wait_for_bit(CTS);
    uint8_t response[7];
    read(i2c_fd, response, 7);
    int freq1 = ((response[2] << 8) | response[3])/100;
    int freq2 = ((response[2] << 8) | response[3])%100/10;
    m_freq = std::to_string(freq1).c_str();
    m_freq += ".";
    m_freq += std::to_string(freq2).c_str();
    m_rssi = response[4];
    m_snr = response[5];
    emit statusUpdated();
    if( (response[1]&0x01) == 0x01) {
        printf("Freq: %d RSSI: %d dBuV SNR: %d dB \n", (response[2] << 8) | response[3], response[4], response[5]);
    }
    return (response[1]&0x80) == 0x00;
}

int Radio::decode_rds() {
    write(i2c_fd, GET_RDS, 2);
    wait_for_bit(CTS);
    uint8_t response[13];
    read(i2c_fd, response, 13);
    int fifo_remaining = response[3];
    int group_type = response[6] >> 3;
    int pty = ((response[6] & 0x03) << 3) | (response[7] >> 5);
    int segment_addr;
    char* first_zero = strchr(m_radio_text_inactive, 0x00);
    char* end = strchr(m_radio_text_inactive, 0x0D);
    bool radio_text_valid = (first_zero > end) && (end != NULL);
    bool text_changed = false;
    switch(group_type) {
        //case 0x00:
        //case 0x01:
            //int segment_addr = response[7] & 0x03;
            //if(m_radio_text) {
                //m_station_name[segment_addr*2] = response[10];
                //m_station_name[segment_addr*2+1] = response[11];
                //}
            //break;
        case 0x04:
            if((response[7] & 0x10) != m_radio_text_ab) {
                memset(m_radio_text_inactive,0,64);
                m_radio_text_ab = response[7] & 0x10;
                radio_text_valid = false;
                printf("AB Switch\n");
            }
            segment_addr = response[7] & 0x0f;
            text_changed |= radio_text_set_char(radio_text_valid,segment_addr*4,response[8]);
            text_changed |= radio_text_set_char(radio_text_valid,segment_addr*4+1,response[9]);
            text_changed |= radio_text_set_char(radio_text_valid,segment_addr*4+2,response[10]);
            text_changed |= radio_text_set_char(radio_text_valid,segment_addr*4+3,response[11]);
            break;
        case 0x05:
            if((response[7] & 0x10) != m_radio_text_ab) {
                memset(m_radio_text_inactive,0,64);
                m_radio_text_ab = response[7] & 0x10;
                radio_text_valid = false;            
            }
            segment_addr = response[7] & 0x0f;
            text_changed |= radio_text_set_char(radio_text_valid,segment_addr*2,response[10]);
            text_changed |= radio_text_set_char(radio_text_valid,segment_addr*2+1,response[11]);
            break;
    }
    first_zero = strchr(m_radio_text_inactive, 0x00);
    end = strchr(m_radio_text_inactive, 0x0D);
    if(!radio_text_valid && (first_zero > end) && (end != NULL)) {
        int length = end - &m_radio_text_inactive[0];
        memset(m_radio_text,0,64);
        memcpy(m_radio_text,m_radio_text_inactive,length);
        text_changed = true;
        printf("Became valid\n");
    }
    if(text_changed) {
        emit radioTextUpdated();
        printf("Radio Text: %s\n",m_radio_text);
    }
    return fifo_remaining;
}

bool Radio::radio_text_set_char(bool radio_text_valid, int i, uint8_t val) {
    if(val == 0x00) {
        return false;
    }
    if(val == 0x0B) {
        val = 0x0A;
    }
    if((val < 0x20 || val > 0x7D) && val != 0x0A && val != 0x0D) {
        val = ' ';
    }
    if(radio_text_valid) {
        if(m_radio_text_inactive[i] == val && m_radio_text_inactive[i] != m_radio_text[i] && val != 0x0D) {
            m_radio_text[i] = val;
            return true;
        } else {
            m_radio_text_inactive[i] = val;
            return false;
        }
    } else {
        m_radio_text_inactive[i] = val;
        return false;
    }
}

QString Radio::radioText() {
    return m_radio_text;
}
