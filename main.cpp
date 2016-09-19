#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "elf.h"

extern "C" {
#include <user_interface.h>
}


const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;

typedef int berr_t;
struct bootloader_args {
    void (*hey)();
    int (*read_adc)();
    berr_t (*http_request)(const char *host, int port, const char *method,
                           const char *path, const void *headers, const void *payload,
                           size_t payload_size, void *buf, size_t buf_size);
};

void hey() {
    Serial.println("hey from app!");
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);
}

int read_adc() {
        return system_adc_read();
}

void println(const char *str) {

    Serial.println(str);
}

enum {
    BERR_OK = 1,
    BERR_CONNECT = 2,
    BERR_NOMEM = 3,
};


berr_t http_request(const char *host, int port, const char *method,
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
            Serial.print("--> ");
            Serial.println(l);
            if (l.equals("\r")) {
                if (prev_is_blank) {
                    in_header = false;
                    continue;
                }
                prev_is_blank = true;
            }
        } else {
            int num;
            Serial.print("fetching");
            while ((num = client.read(p, size)) > 0) {
                Serial.print("fetched ");
                Serial.println(num);
                p    += num;
                size -= num;

                if (size < 0) {
                    Serial.println("http: buffer is too short");
                    return BERR_NOMEM;
                }
            }
        }
    }

    int num;
    Serial.println(num = client.read(p, size));
    p    += num;
    size -= num;
    Serial.println(client.read(p, size));
    ESP.wdtFeed();
    if (payload_size > 0)
        return 0;

    return BERR_OK;
}


void do_update() {
    char buf[1024];
    size_t buf_size = sizeof(buf);
    String path("/devices/");
    path.concat(DEVICE_NAME);
    path.concat("/image");

    http_request(CODESTAND_HOST, CODESTAND_PORT, "GET", path.c_str(),
                 "", "", 0, &buf, buf_size);

    struct bootloader_args bargs;
    bargs.hey = hey;
    bargs.http_request = http_request;
    bargs.read_adc = read_adc;

    void (*app)(struct bootloader_args *);
    app = (void (*)(struct bootloader_args *)) load_elf(&buf, nullptr);
    app(&bargs);
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
