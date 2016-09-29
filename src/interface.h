#ifndef __INTERFACE_H__
#define __INTERFACE_H__

int gpio_read(int pin);
void gpio_write(int pin, int data);
void gpio_set_pin_mode(int pin, int mode);
void dprint(const char *msg, unsigned x);
void printchar(const char ch);
int read_adc();

#endif
