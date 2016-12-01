#include <Arduino.h>
#include "http.h"
#include "update.h"
#include "reset_stack_and_goto.h"
#include "elf.h"
#include "consts.h"
#include "interface.h"
#include "interrupt.h"
#include "i2c.h"


static unsigned long current_deployment_id = 0;
static struct firmware_info finfo;

void do_update() {
    finfo.update            = update;
    finfo.delay             = delay;
    finfo.dprint            = dprint;
    finfo.printchar         = printchar;
    finfo.read_adc          = read_adc;
    finfo.analog_write      = analog_write;
    finfo.gpio_read         = gpio_read;
    finfo.gpio_write        = gpio_write;
    finfo.gpio_set_pin_mode = gpio_set_pin_mode;
    finfo.http_request      = _http_request;
    finfo.get_device_secret = get_device_secret;
    finfo.get_server_url    = get_server_url;
    finfo.get_deployment_id = get_deployment_id;
    finfo.accept_interrupt  = accept_interrupt;
    finfo.i2c_send          = i2c_send;
    finfo.i2c_receive       = i2c_receive;
    finfo.millis            = millis;

    ESP.wdtFeed();

    void (*app)(struct firmware_info *);
    app = (void (*)(struct firmware_info *)) load_elf(current_deployment_id);

    ESP.wdtFeed();

    Serial.println("firmware: starting the app");
    app(&finfo);
    Serial.println("firmware: BUG: the app returned");

    for (;;);
}


void update(unsigned long deployment_id) {

    current_deployment_id = deployment_id;
    reset_stack_and_goto(do_update);
}


void send_first_heartbeat() {

send:
    ESP.wdtFeed();

    String url(SERVER_URL);
    url.concat("/api/devices/");
    url.concat(DEVICE_SECRET);
    url.concat("/heartbeat?status=ready");

    size_t resp_size;
    char buf[64];
    memset(&buf, 0, sizeof(buf));
    http_request("PUT", url, "", "", 0, &buf, sizeof(buf), &resp_size);

    if (buf[0] == 'X') {
        Serial.println("firmware: no deployments");
        goto retry;
    }

    current_deployment_id = atol((const char *) &buf);
    if (current_deployment_id == 0) {
        Serial.println("firmware: server sent invalid heartbeat response");
        goto retry;
    }

    Serial.print("firmware: starting deplyoment id #");
    Serial.println(current_deployment_id);
    update(current_deployment_id);

retry:
    Serial.println("firmware: retrying in few seconds...");
    ESP.wdtFeed();
    delay(3000);
    goto send;
}


unsigned long get_deployment_id() {

    return current_deployment_id;
}
