#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <cstdint>

//std::vector<uint8_t> i2cRead(int i2c_fd, uint8_t i2cAddr, uint8_t chipAddr, int numBytes) { return i2cRead(i2c_fd,i2cAddr,{chipAddr},numBytes); }
std::vector<uint8_t> i2cRead(int i2c_fd, uint8_t i2cAddr, std::vector<uint8_t> chipAddr, int numBytes);

#endif 
