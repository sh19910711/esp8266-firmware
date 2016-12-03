#include <twi.h>
#include "i2c.h"


void i2c_init() {

    twi_init(SDA, SCL);
}


void i2c_init(int sda, int scl) {

    twi_init(sda, scl);
}


void i2c_send(int addr, const void *buf, size_t size) {

    twi_writeTo(addr, (unsigned char *) buf, size, true);
}


void i2c_receive(int addr, void *buf, size_t size) {

    twi_readFrom(addr, (unsigned char *) buf, size, true);
}

