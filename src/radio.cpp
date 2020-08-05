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
const uint8_t REFCLK_FREQ[] = {0x12, 0x00, 0x02, 0x01, 0x7D, 0x00};
const uint8_t REFCLK_PRESCALE[] = {0x12, 0x00, 0x02, 0x02, 0x00, 0x60};
const uint8_t TUNE_START[] = {0x20, 0x00, 0x22, 0x2E, 0x00};
const uint8_t SEEK_NOWRAP[] = {0x21, 0x08};
const uint8_t GET_INT_STATUS[] = {0x14};
const uint8_t FM_TUNE_STATUS[] = {0x22, 0x00};
const int CTS = 7;
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
    char filename[20];

    snprintf(filename, 19, "/dev/i2c-%d", 6);
    i2c_fd = open(filename, O_RDWR);
    if (i2c_fd < 0) {
        exit(1);
    }
    if (ioctl(i2c_fd, I2C_SLAVE, 0x11) < 0) {
        exit(1);
    }
    printf("Radio Power\n");
    write(i2c_fd, PWR_UP, 3);
    wait_for_bit(CTS);
    printf("Radio clk\n");
    write(i2c_fd, REFCLK_FREQ, 6);
    wait_for_bit(CTS);
    write(i2c_fd, REFCLK_PRESCALE, 6);
    {
        using namespace std::chrono;
        std::this_thread::sleep_for(500ms);
    }
    printf("Tune Start\n");
    write(i2c_fd, TUNE_START, 5);
    wait_for_bit(STCINT);
    while(decode_fm_tune_status()) {
//        printf("Seek\n");
        write(i2c_fd, SEEK_NOWRAP, 2);
        wait_for_bit(STCINT);
    } 

}

bool Radio::decode_fm_tune_status() {
    write(i2c_fd, FM_TUNE_STATUS, 2);
    wait_for_bit(CTS);
    uint8_t response[7];
    read(i2c_fd, response, 7);
//    printf("%#02x %#02x %#02x %#02x %#02x %#02x %#02x\n", response[0], response[1], response[2], response[3], response[4], response[5],response[6] );
    if( (response[1]&0x01) == 0x01) {
        printf("Freq: %d RSSI: %d dBuV SNR: %d dB \n", (response[2] << 8) | response[3], response[4], response[5]);
    }   
    return (response[1]&0x80) == 0x00;
}   
