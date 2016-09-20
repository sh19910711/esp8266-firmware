#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "elf.h"
#include "finfo.h"

extern "C" {
#include <user_interface.h>
}

const char* device_name = DEVICE_NAME;
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;


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
                if (*buf_size < num) {
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
    

    // XXX
    int offset = 0;
    while (buf_size > 0) {
        ESP.wdtFeed();
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

    return BERR_NOMEM;
}

static struct firmware_info finfo;
void do_update() {
    String path("/devices/");
    path.concat(device_name);
    path.concat("/image");

    char *buf = (char *) 0x3fff9000;
    size_t buf_size = 0x3000;

    http_request(CODESTAND_HOST, CODESTAND_PORT, "GET", path.c_str(),
                 "", "", 0, buf, buf_size);

    finfo.dprint       = dprint;
    finfo.printchar    = printchar;
    finfo.read_adc     = read_adc;
    finfo.http_request = http_request;

    ESP.wdtFeed();

    void (*app)(struct firmware_info *);
    app = (void (*)(struct firmware_info *)) load_elf(buf, nullptr);

    ESP.wdtFeed();

    Serial.println("firmware: starting the app");
    app(&finfo);
    Serial.println("firmware: BUG: the app returned");
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
    do_update();
}


void loop() {
}
