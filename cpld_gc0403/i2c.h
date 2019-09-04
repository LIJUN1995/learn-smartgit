#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#define I2C_SLAVER_WRITE_DEVICE_ID 0x78
#define I2C_SLAVER_READ_DEVICE_ID 0x79

int i2c_write_data(unsigned short reg_address, unsigned char reg_data);
char i2c_read_data(uint16_t reg_address);
#endif

