#include <twi.h>
#include "i2c.h"


void i2c_init() {

    twi_init(4 /* sda */, 5 /* scl */);
}


void i2c_send(int addr, const void *buf, size_t size) {

    twi_writeTo(addr, (unsigned char *) buf, size, true);
}


void i2c_receive(int addr, void *buf, size_t size) {

    twi_readFrom(addr, (unsigned char *) buf, size, true);
}

