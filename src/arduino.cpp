#include "arduino.h"
#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>    // POSIX terminal control definitions
#include <iostream>

#include <thread>

const int MAX_LINE_LENGTH = 100;

void setBaud2(int uart_fd,speed_t speed) {
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

void Arduino::run() {
    {
        const std::lock_guard<std::mutex> lg(writeMutex);
        uart_fd = open("/dev/ttyAMA1", O_RDWR| O_NOCTTY );
        setBaud2(uart_fd, B9600);
        write(uart_fd, " \n", 2);
    }
    char current_line[MAX_LINE_LENGTH + 1];
    int position = 0;
    while(true) {
        position += read( uart_fd, current_line + position, 1 );
        position %= MAX_LINE_LENGTH;
        if( position == 0 || current_line[position-1] != '\n' ) {
            continue;
        }
        current_line[position-1] = 0x00;
        position = 0;
        printf("Arduino: %s\n", current_line);
        fflush(stdout);
        QString str = current_line;
        emit msg(str);
    }
}

void Arduino::sendCmd(QString cmd) {
    const std::lock_guard<std::mutex> lg(writeMutex);
    cmd = cmd.append("\n");
    const char* str = cmd.toStdString().c_str();
    write(uart_fd, str, cmd.length());   
}

Arduino::Arduino (QObject *parent ) {
    auto lambda = [](Arduino* a) {a->run();};
    std::thread t(lambda, this);
    m_thread.swap(t);
}
