#include "util.h"
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

std::vector<uint8_t> i2cRead(int i2c_fd, uint8_t i2cAddr, std::vector<uint8_t> chipAddr, int numBytes) {
    std::vector<uint8_t> readData(numBytes);
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset[1];
    msgs[0].addr = i2cAddr;
    msgs[0].flags = 0x00;
    msgs[0].len = chipAddr.size();
    msgs[0].buf = chipAddr.data();
    msgs[1].addr = i2cAddr;
    msgs[1].flags = I2C_M_RD | I2C_M_NOSTART;
    msgs[1].len = numBytes;
    msgs[1].buf = readData.data();
    msgset[0].msgs = msgs;
    msgset[0].nmsgs = 2;
    ioctl(i2c_fd, I2C_RDWR, &msgset);
    for(int i=0;i<numBytes;i++) {
        readData[i] = msgset[0].msgs[1].buf[i];
    }
    return readData;
}


