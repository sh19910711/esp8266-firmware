#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "elf.h"
#include "finfo.h"

extern "C" {
#include <user_interface.h>
}

extern "C" void reset_stack_and_goto(void (*func)());
void set_loop(void (*func)());

const char* board_name = "esp8266";
const char* device_name = DEVICE_NAME;
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;

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


void println(const char *str) {

    Serial.println(str);
}


static ferr_t do_http_request(const char *host, int port, const char *method,
                              const char *path, const void *headers, const void *payload,
                              size_t payload_size, uint8_t **buf, size_t *buf_size,
                              int *offset) {

    if (*buf_size == 0)
        return BERR_NOMEM;

    Serial.print("http: ");
    Serial.print(method);
    Serial.print(" ");
    Serial.print(host);
    Serial.print(" ");
    Serial.print(path);
    Serial.print(" offset=");
    Serial.println(*offset);

    WiFiClient client;
    if (!client.connect(host, port)) {
        println("error: failed to connect");
        return BERR_CONNECT;
    }

    client.print(String(method) + " " + path + " HTTP/1.0\r\n" +
                 "Host: " + host + "\r\n" +
                 "Range: bytes=" + String(*offset) + "-" + "\r\n" +
                 "Connection: close\r\n" +
                 "\r\n");

    // TODO: support headers and payload

    while (!client.available()) {
        // connecting
        // TODO: implement timeout
    }

    ESP.wdtFeed();

    bool in_header = true;
    while (client.available()) {
        // connected
        // TODO: support Content-Length
        // TODO: support redirection
        if (in_header) {
            String l = client.readStringUntil('\n');
            if (l.startsWith("X-End-Of-File:")) {
                return BERR_EOF;
            }
            
            if (l.equals("\r")) {
                in_header = false;
            }
        } else {
            size_t num;
            while ((num = client.read(*buf, *buf_size)) > 0) {
                Serial.println(num);
                *offset += num;
                if (*buf_size == num) {
                    Serial.println("http: buffer is too short");
                    return BERR_NOMEM;
                }

                *buf += num;
                *buf_size -= num;
            }
        }
    }

    ESP.wdtFeed();
    return BERR_OK;
}


static ferr_t http_request(const char *host, int port, const char *method,
                           const char *path, const void *headers, const void *payload,
                           size_t payload_size, void *buf, size_t buf_size) {

    int offset = 0;
    ferr_t r = do_http_request(host, port, method, path, headers,
                               payload, payload_size,
                              (uint8_t **) &buf, &buf_size,
                               &offset);

    return r;
}


static ferr_t xeof_http_request(const char *host, int port, const char *method,
                                const char *path, const void *headers, const void *payload,
                                size_t payload_size, void *buf, size_t buf_size) {

    int offset = 0;
    for (;;) {
        ferr_t r = do_http_request(host, port, method, path, headers,
                                   payload, payload_size,
                                   (uint8_t **) &buf, &buf_size,
                                   &offset);

        switch (r) {
        case BERR_EOF:
            return BERR_OK;
        case BERR_OK:
            continue;
        default:
            return r;
        }
    }

    // unreachable
}


static struct firmware_info finfo;
void do_update() {
    String path("/devices/");
    path.concat(device_name);
    path.concat("/image");

    char *buf = (char *) 0x3fff9000;
    size_t buf_size = 0x3000;

    xeof_http_request(CODESTAND_HOST, CODESTAND_PORT, "GET", path.c_str(),
                 "", "", 0, buf, buf_size);

    finfo.set_loop     = set_loop;
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


static int current_deployment_id = 0;
void update_status() {

retry:
    String path("/devices/");
    path.concat(device_name);
    path.concat("/status");

    path.concat("?board=");
    path.concat(board_name);
    path.concat("&status=");
    if (current_deployment_id == 0) {
        path.concat("ready");
    } else {
        path.concat("running");
    }

    char buf[64];
    http_request(CODESTAND_HOST, CODESTAND_PORT, "PUT", path.c_str(),
                 "", "", 0, &buf, sizeof(buf));

    if (buf[0] != 'X') {
        int latest = atol(buf);
        if (current_deployment_id != latest) {
            Serial.println("firmware: new deployment detected, updating...");
            current_deployment_id = latest;
            reset_stack_and_goto(do_update);
        }
    } else {
        Serial.println("firmware: no deployments, retrying...");
        delay(5000);
        goto retry;
    }
}


#include "Schedule.h"
extern "C" void __yield();
static void (*app_loop)() = nullptr;
void set_loop(void (*func)()) {

    Serial.println("we are in set_loop");
    app_loop = func;

    for (;;) {
        Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
        run_scheduled_functions();
        __yield();
        ESP.wdtFeed();
        if (app_loop)
            app_loop();

        update_status();
    }
}


void loop() {
    // unused
}


extern "C" void boot() {
    Serial.begin(115200);
    Serial.println();

    Serial.print("firmware: connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("firmware: connected");
    Serial.print("firmware: my IP address is ");
    Serial.println(WiFi.localIP());

    Serial.println("firmware: activating IRAM from 0x40108000");
    *((uint32_t *) 0x3ff00024) |= 0x10;

    Serial.println("firmware: downloading an app");
    update_status();
}
