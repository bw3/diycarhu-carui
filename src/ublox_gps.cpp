#include "ublox_gps.h"
#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions
#include <iostream>
#include <time.h>

const int MAX_LINE_LENGTH = 100;

void setBaud(int uart_fd,speed_t speed) {
    struct termios tty;
    memset (&tty, 0, sizeof tty);
/*    if ( tcgetattr ( uart_fd, &tty ) != 0 ) {
        std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
    }*/
    cfsetospeed (&tty, (speed_t)speed);
    cfsetispeed (&tty, (speed_t)speed);

    /* Setting other Port Stuff */
    tty.c_cflag     &=  ~PARENB;            // Make 8n1
    tty.c_cflag     &=  ~CSTOPB;
    tty.c_cflag     &=  ~CSIZE;
    tty.c_cflag     |=  CS8;

    tty.c_cflag     &=  ~CRTSCTS;           // no flow control
    tty.c_cc[VMIN]   =  1;                  // read doesn't block
    tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
    tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

    /* Make raw */
    cfmakeraw(&tty);


    tcdrain( uart_fd);
    if ( tcsetattr ( uart_fd, TCSANOW, &tty ) != 0) {
        std::cout << "Error " << errno << " from tcsetattr" << std::endl;
    }
}

void sendUBX(int uart_fd, uint8_t cl, uint8_t id, uint16_t length, const uint8_t* payload) {
    uint8_t header[6] = {0xB5, 0x62, cl, id, (uint8_t) length, (uint8_t) (length>>8) };
    uint8_t ck_a=0, ck_b=0;
    for(int i=2;i<6;i++) {
        ck_a += header[i];
        ck_b += ck_a;
    }
    for(int i=0;i<length;i++) {
        ck_a += payload[i];
        ck_b += ck_a;
    }
    write(uart_fd, header, 6);
    write(uart_fd, payload, length);
    write(uart_fd, &ck_a, 1);
    write(uart_fd, &ck_b, 1);
}

void cfgRate(int uart_fd, uint16_t measRate, uint16_t navRate, uint16_t timeRef) {
    uint8_t payload[6] = {
        (uint8_t) (measRate),
        (uint8_t) (measRate >> 8),
        (uint8_t) (navRate),
        (uint8_t) (navRate >> 8),
        (uint8_t) (timeRef),
        (uint8_t) (timeRef >> 8)
    };
    sendUBX(uart_fd, 0x06, 0x08, 6, payload);
}

void cfgPrt(int uart_fd, uint32_t baud_rate) {
    uint8_t payload[20] = {
        0x01, //uart
        0x00,
        0x00,
        0x00,
        0xD0, //8bit
        0x08, //No parity, 1 stop bit
        0x00,
        0x00,
        (uint8_t) (baud_rate),
        (uint8_t) (baud_rate >> 8),
        (uint8_t) (baud_rate >> 16),
        (uint8_t) (baud_rate >> 24),
        0x07,
        0x00,
        0x03,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00
    };
    sendUBX(uart_fd, 0x06, 0x00, 20, payload);
}

void cfgRxm(int uart_fd) {
    uint8_t payload[2] = {
        0x00, 
        0x01 // Power save mode
    };
    sendUBX(uart_fd, 0x06, 0x11, 2, payload);
}

void cfgPm2(int uart_fd) {
    const uint8_t maxStartupStateDur = 60; // s
    const uint32_t updatePeriod = 60*60; //ms
    const uint32_t searchPeriod = 0; //ms
    const uint32_t gridOffset = 0; //ms
    const uint16_t  onTime = 60; //s
    const uint16_t  minAcqTime = 0; //s
    const uint32_t  extintInactivityMs = 0; //ms
    uint8_t payload[48] = {
        0x02, //version
        0x00, 
        maxStartupStateDur,
        0x00,
        0x20, //extintWake
        0x18,//0x18, //updateEPH, updateRTC
        0x00, //mode
        0x00,
        (uint8_t) (updatePeriod << 0),(uint8_t) (updatePeriod << 8),(uint8_t) (updatePeriod << 16),(uint8_t) (updatePeriod << 24),
        (uint8_t) (searchPeriod << 0),(uint8_t) (updatePeriod << 8),(uint8_t) (updatePeriod << 16),(uint8_t) (updatePeriod << 24),
        (uint8_t) (gridOffset << 0),(uint8_t) (gridOffset << 8),(uint8_t) (gridOffset << 16),(uint8_t) (gridOffset << 24),
        (uint8_t) (onTime << 0),(uint8_t) (onTime << 8),
        (uint8_t) (minAcqTime << 0),(uint8_t) (minAcqTime << 8),
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        (uint8_t) (extintInactivityMs << 0),(uint8_t) (extintInactivityMs << 8),(uint8_t) (extintInactivityMs << 16),(uint8_t) (extintInactivityMs << 24)
    };
    sendUBX(uart_fd, 0x06, 0x3B, 48, payload);
}

void cfgTp5(int uart_fd, uint8_t timepulse) {
    // disable timepulse
    uint8_t payload[32] = {
        timepulse,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00
    };
    sendUBX(uart_fd, 0x06, 0x31, 32, payload);
}

void rxmPmreq(int uart_fd) {
        uint8_t payload[16] = {
        0x00, //clear
        0x00,
        0x00,
        0x00,
        0x00, //save
        0x00,
        0x00,
        0x00,
        0x02, //load
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00
    };
    sendUBX(uart_fd, 0x02, 0x41, 16, payload);
}

void cfgPms(int uart_fd) {
    const uint16_t period = 60*60;
    const uint16_t onTime = 30;
    uint8_t payload[8] = {
        0x00,
        0x02,
        (uint8_t) period,
        (uint8_t) period >> 8,
        (uint8_t) onTime,
        (uint8_t) onTime >> 8,
        0x00,
        0x00
    };
    sendUBX(uart_fd, 0x06, 0x86, 8, payload);
}

void cfgAnt(int uart_fd) {
    uint8_t payload[4] = {
        0x1A,
        0x00,
        0xF0,
        0xB9
    };
    sendUBX(uart_fd, 0x06, 0x13, 4, payload);
}

void cfgReset(int uart_fd) {
    uint8_t payload[12] = {
        0x1E, //clear
        0x1F,
        0x00,
        0x00,
        0x00, //save
        0x00,
        0x00,
        0x00,
        0x00, //load
        0x00,
        0x00,
        0x00
    };
    sendUBX(uart_fd, 0x06, 0x09, 12, payload);
}


void cfgSave(int uart_fd) {
    uint8_t payload[12] = {
        0x00, //clear
        0x00,
        0x00,
        0x00,
        0x10, //save
        0x04,
        0x00,
        0x00, 
        0x00, //load
        0x00,
        0x00,
        0x00
    };
    sendUBX(uart_fd, 0x06, 0x09, 12, payload);
}

void configNMEA(int uart_fd) {
    cfgRate(uart_fd,50,1,0);
    const uint8_t RMC_RATE[3] = {0xF0, 0x04, 1};
    sendUBX(uart_fd, 0x06, 0x1, 3, RMC_RATE);
    const uint8_t GSV_RATE[3] = {0xF0, 0x03, 50};
    sendUBX(uart_fd, 0x06, 0x1, 3, RMC_RATE);
}

void UbloxGps::runRecv() {
    char current_line[MAX_LINE_LENGTH + 1];
    int position = 0;
    while(true) {
        position += read( uart_fd, current_line + position, 1 );
        position %= MAX_LINE_LENGTH;
        if( current_line[0] != '$' && current_line[0] != 0xB5) {
            position = 0;
            continue;
        }
        if( position > 7  && current_line[0] == 0xB5) {
            for(int i=0;i<position;i++)
                printf("%x ",current_line[i]);
            printf("\n");
            position = 0;
            continue;
        }
        if( position <3 || current_line[position-3] != '*') {
            continue;
        }
        current_line[position] = 0;
        uint8_t stated_checksum = strtol( current_line + position - 2, NULL, 16);
        uint8_t calc_checksum = 0;
        for(int i=1; i< position-3 ; i++) {
            calc_checksum ^= current_line[i];
        }
        if(stated_checksum != calc_checksum) {
            position = 0;
            continue;
        }
        current_line[position-3] = 0;
        decodeNMEA( current_line + 1 );
        position=0;
    }
}

char* next_field(char** saveptr) {
    char* start = *saveptr;
    char* end = strchr(*saveptr,',');
    if( end != NULL ) {
        *end = '\0';
        *saveptr = end+1;
    }
    return start;
}

void UbloxGps::decodeNMEA(char* str) {
    char** saveptr = &str;
    char* msgtype = next_field(saveptr);
    if(strstr(msgtype, "RMC")) {
    //    printf("RMC\n");
        double fix_time = strtod(next_field(saveptr), NULL);
        bool active = next_field(saveptr)[0] == 'A';
        char* latitude_raw = next_field(saveptr);
        char* latitude_dir = next_field(saveptr);
        char* longitude_raw = next_field(saveptr);
        char* longitude_dir = next_field(saveptr);
        double speed = strtod(next_field(saveptr), NULL);
        char* bearing_raw = next_field(saveptr);
        long  fix_date = strtol(next_field(saveptr), NULL, 10);
        if(!clock_set && fix_date != 0) {
            char date_time_str[30];
            snprintf(date_time_str,29,"20%02d-%02d-%02d %02d:%02d:%02d\n", 
                (int)fix_date%100, 
                (int)fix_date/100%100, 
                (int)fix_date/100/100%100, 
                (int)(fix_time+0.5)/100/100%100, 
                (int)(fix_time+0.5)/100%100,
                (int)(fix_time+0.5)%100
            );
            printf(date_time_str);
            struct tm t;
            t.tm_year = (int)fix_date%100+100;
            t.tm_mon = (int)fix_date/100%100-1;
            t.tm_mday = (int)fix_date/100/100%100;
            t.tm_hour = (int)(fix_time+0.5)/100/100%100;
            t.tm_min = (int)(fix_time+0.5)/100%100;
            t.tm_sec = (int)(fix_time+0.5)/100;
            printf("Minutes %d\n",t.tm_min);
            struct timespec epoch_spec;
            epoch_spec.tv_sec = timegm(&t);
            printf("Epoch: %ld", epoch_spec.tv_sec);
            clock_settime(CLOCK_REALTIME,&epoch_spec);
            clock_set = true;
        }
        if(active) {
            double latitude = (strtol(latitude_raw, NULL, 10) / 100) + strtod(latitude_raw+2,NULL) / 60;
            if( latitude_dir[0] == 'S') {
                latitude *= -1;
            }
            double longitude = (strtol(longitude_raw, NULL, 10) / 100) + strtod(longitude_raw+3,NULL) / 60;
            if( longitude_dir[0] == 'W') {
                longitude*= -1;
            }
            double bearing = strtod(bearing_raw, NULL);
            setPosition(latitude,longitude);
            if(bearing_raw[0] != '\0') {
                setBearing(bearing);
            }
        }
    } else {
      //  printf(msgtype);
        //printf("\n");
    }
}

void UbloxGps::runSetup() {
    printf("open\n");
    uart_fd = open("/dev/ttyAMA2", O_RDWR| O_NOCTTY );
    printf("setBaud\n");
    setBaud(uart_fd, B9600);
    cfgReset(uart_fd);
    printf("cfgPrt\n");
    cfgPrt(uart_fd, 115200);
    printf("setBaud\n");
    setBaud(uart_fd, B115200);
    auto lambda = [](UbloxGps* g) {g->runRecv();};
    std::thread t(lambda, this);
    std::this_thread::sleep_for(std::chrono::milliseconds(2100));
    configNMEA(uart_fd);
    cfgPm2(uart_fd);
    cfgTp5(uart_fd,0);
    cfgTp5(uart_fd,1);
//    cfgPms(uart_fd);
    cfgAnt(uart_fd);
    cfgRxm(uart_fd);
    cfgSave(uart_fd);
//    rxmPmreq(uart_fd);
    t.join();
}

UbloxGps::UbloxGps (QObject *parent ) : Gps(parent), clock_set(false) {
    auto lambda = [](UbloxGps* g) {g->runSetup();};
    static std::thread t(lambda, this);
//    t.join();
}
