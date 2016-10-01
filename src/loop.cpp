#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Schedule.h>
#include "update.h"
#include "loop.h"

extern "C" {
#include <user_interface.h>
}


struct timer timers;

void set_interval(int ms, void (*callback)()) {

    timers.callback = callback;
}


void start_loop() {

    for (;;) {
        ESP.wdtFeed();
        timers.callback();
        update_status();
        delay(GLOBAL_INTERVAL);
    }
}


void loop() {
    // unused
}
