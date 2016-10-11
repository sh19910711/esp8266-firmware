#include <Arduino.h>
#include "http.h"
#include "update.h"
#include "reset_stack_and_goto.h"
#include "elf.h"
#include "loop.h"
#include "consts.h"
#include "interface.h"


static int current_deployment_id = 0;
static struct firmware_info finfo;

void do_update() {
    String path("/api/devices/");
    path.concat(device_rand_id);
    path.concat("/image?deployment_id=");
    path.concat(String(current_deployment_id));

    char *buf = (char *) 0x3fff9000; // XXX
    size_t buf_size = 0x3000; // XXX

    xeof_http_request(CODESTAND_HOST, CODESTAND_PORT, "GET", path.c_str(),
                 "", "", 0, buf, buf_size);

    finfo.start_loop   = start_loop;
    finfo.set_interval = set_interval;
    finfo.dprint       = dprint;
    finfo.printchar    = printchar;
    finfo.read_adc     = read_adc;
    finfo.gpio_read    = gpio_read;
    finfo.gpio_write    = gpio_write;
    finfo.gpio_set_pin_mode = gpio_set_pin_mode;
    finfo.http_request = http_request;

    ESP.wdtFeed();

    void (*app)(struct firmware_info *);
    app = (void (*)(struct firmware_info *)) load_elf(buf, nullptr);

    ESP.wdtFeed();

    Serial.println("firmware: starting the app");
    app(&finfo);
    Serial.println("firmware: BUG: the app returned");

    for (;;);
}


void update_status() {

retry:
    ESP.wdtFeed();

    String path("/api/devices/");
    path.concat(device_rand_id);
    path.concat("/status");

    path.concat("&status=");
    if (current_deployment_id == 0) {
        path.concat("ready");
    } else {
        path.concat("running");
    }

    char buf[64];
    memset(&buf, 0, sizeof(buf));
    http_request(CODESTAND_HOST, CODESTAND_PORT, "PUT", path.c_str(),
                 "", "", 0, &buf, sizeof(buf));

    if (buf[0] != 'X') {
        int latest = atol((const char *) &buf);
        if (current_deployment_id < latest) {
            Serial.print("firmware: new deployment detected, updating... (#");
            Serial.print(current_deployment_id);
            Serial.print(" -> #");
            Serial.print(latest);
            Serial.println(")\n");
            current_deployment_id = latest;
            reset_stack_and_goto(do_update);
        }
    } else {
        Serial.println("firmware: no deployments, retrying...");
        ESP.wdtFeed();
        delay(3000);
        goto retry;
    }
}
