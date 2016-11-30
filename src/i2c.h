#ifndef __I2C_H__
#define __I2C_H__

#include <Arduino.h>

void i2c_init();
void i2c_send(int addr, const void *buf, size_t size);
void i2c_receive(int addr, void *buf, size_t size);

#endif
