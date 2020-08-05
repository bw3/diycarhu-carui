#include "amp.h"

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


Amp::Amp (QObject *parent ) { 
printf("loading amp ... \n");
  int _i2c_fd;
  char filename[20];
  snprintf(filename, 19, "/dev/i2c-%d", 6);
  _i2c_fd = open(filename, O_RDWR);
  if (_i2c_fd < 0) {
    exit(1);
  }
  if (ioctl(_i2c_fd, I2C_SLAVE, 0x6B) < 0) {
    exit(1);
  }
  uint16_t bytesSent = 0;
  std::vector<uint8_t> writeData = {0x00, 0x00, 0x31, 0x62, 0x44, 0x00,0xA0, 0xA0, 0xA0, 0xA0};
    write(_i2c_fd, writeData.data(), writeData.size());

}

