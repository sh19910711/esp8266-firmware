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

void update(int deployment_id);

void do_update() {
    finfo.start_loop   = start_loop;
    finfo.update       = update;
    finfo.set_interval = set_interval;
    finfo.dprint       = dprint;
    finfo.printchar    = printchar;
    finfo.read_adc     = read_adc;
    finfo.gpio_read    = gpio_read;
    finfo.gpio_write    = gpio_write;
    finfo.gpio_set_pin_mode = gpio_set_pin_mode;
    finfo.http_request = http_request;
    finfo.get_device_secret = get_device_secret;
    finfo.get_server_url = get_server_url;

    ESP.wdtFeed();

    void (*app)(struct firmware_info *);
    app = (void (*)(struct firmware_info *)) load_elf(current_deployment_id);

    ESP.wdtFeed();

    Serial.println("firmware: starting the app");
    app(&finfo);
    Serial.println("firmware: BUG: the app returned");

    for (;;);
}


void update(int deployment_id) {

    if (deployment_id > 0) {
        current_deployment_id = deployment_id;
    }

    reset_stack_and_goto(do_update);
}


void send_heartbeat() {

retry:
    ESP.wdtFeed();

    String path("/api/devices/");
    path.concat(DEVICE_SECRET);
    path.concat("/heartbeat?");

    path.concat("status=");
    if (current_deployment_id == 0) {
        path.concat("ready");
    } else {
        path.concat("running");
    }

    char buf[64];
    memset(&buf, 0, sizeof(buf));
    http_request(SERVER_HOST, SERVER_PORT, "PUT", path.c_str(),
                 "", "", 0, &buf, sizeof(buf), SERVER_TLS);

    if (buf[0] != 'X') {
        int latest = atol((const char *) &buf);
        if (current_deployment_id < latest) {
            Serial.print("firmware: new deployment detected, updating... (#");
            Serial.print(current_deployment_id);
            Serial.print(" -> #");
            Serial.print(latest);
            Serial.println(")\n");
            update(latest);
        }
    } else {
        Serial.println("firmware: no deployments, retrying...");
        ESP.wdtFeed();
        delay(3000);
        goto retry;
    }
}
