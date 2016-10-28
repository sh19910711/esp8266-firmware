#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Schedule.h>
#include "update.h"
#include "loop.h"

extern "C" {
#include <user_interface.h>
}


struct timer timers[TIMERS_MAX];

void set_interval(int ms, void (*callback)()) {

    for (int i = 0; i < TIMERS_MAX; i++) {
        if (timers[i].callback == nullptr) {
            timers[i].callback = callback;
            return;
        }
    }

    Serial.println("too many timer timers");
}


void start_loop() {

    for (;;) {
        ESP.wdtFeed();

        for (int i = 0; i < TIMERS_MAX; i++) {
            // TODO
            if (timers[i].callback)
                timers[i].callback();
        }

        delay(GLOBAL_INTERVAL);
    }
}


void init_timers() {

    for (int i = 0; i < TIMERS_MAX; i++)
        timers[i].callback = nullptr;
}


void loop() {
    // unused
}
