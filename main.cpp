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


void printchar(const char ch) {
    Serial.print((char) ch);
}


int read_adc() {
        return system_adc_read();
}


void println(const char *str) {

    Serial.println(str);
}


ferr_t http_request(const char *host, int port, const char *method,
                    const char *path, const void *headers, const void *payload,
                    size_t payload_size, void *buf, size_t buf_size) {

    Serial.print("http_request: starting a http request to ");
    Serial.println(host);

    WiFiClient client;
    if (!client.connect(host, port)) {
        println("error: failed to connect");
        return BERR_CONNECT;
    }

    client.print(String(method) + " " + path + " HTTP/1.0\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n" +
                 "\r\n");

    // TODO: support headers and payload

    while (!client.available()) {
        // connecting
        // TODO: implement timeout
    }

    ESP.wdtFeed();

    uint8_t* p = (uint8_t *) buf;
    size_t size = buf_size;
    bool in_header = true;
    bool prev_is_blank = true;
    while (client.available()) {
        // connected
        // TODO: support Content-Length
        // TODO: support redirection
        if (in_header) {
            String l = client.readStringUntil('\n');
            if (l.equals("\r")) {
                if (prev_is_blank) {
                    in_header = false;
                    continue;
                }
                prev_is_blank = true;
            }
        } else {
            size_t num;
            while ((num = client.read(p, size)) > 0) {
                if (size < num) {
                    Serial.println("http: buffer is too short");
                    return BERR_NOMEM;
                }

                p    += num;
                size -= num;
            }
        }
    }

    ESP.wdtFeed();
    return BERR_OK;
}


static struct firmware_info finfo;
void do_update() {
    String path("/devices/");
    path.concat(device_name);
    path.concat("/image");

    char *buf = (char *) 0x3fff7000;
    size_t buf_size = 0x5000;

    http_request(CODESTAND_HOST, CODESTAND_PORT, "GET", path.c_str(),
                 "", "", 0, buf, buf_size);

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
