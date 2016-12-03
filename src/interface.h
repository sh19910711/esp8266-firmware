#ifndef __INTERFACE_H__
#define __INTERFACE_H__

int gpio_read(int pin);
void gpio_write(int pin, int data);
void gpio_set_pin_mode(int pin, int mode);
void dprint(const char *msg, unsigned x);
void printchar(const char ch);
int read_adc();
void analog_write(int pin, int value);
const char *get_device_secret();
const char *get_server_url();
int wifi_enable_eap_tls(const char *cert, const char *key);

#endif
