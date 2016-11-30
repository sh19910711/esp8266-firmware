#include <Arduino.h>
#include "consts.h"

extern "C" {
#include <user_interface.h>
}

int gpio_read(int pin) {

    return digitalRead(pin);
}


void gpio_write(int pin, int data) {

    digitalWrite(pin, data);
}


void gpio_set_pin_mode(int pin, int mode) {

    pinMode(pin, mode);
}


void dprint(const char *msg, unsigned x) {

    Serial.print(msg);
    Serial.print(" 0x");
    Serial.println(x, HEX);
}


void printchar(const char ch) {

    Serial.print((char) ch);
}


int read_adc() {
        return system_adc_read();
}


void analog_write(int pin, int value) {

    analogWrite(pin, value);
}


const char *get_device_secret() {

    return DEVICE_SECRET;
}


const char *get_server_url() {

    return SERVER_URL.c_str();
}
