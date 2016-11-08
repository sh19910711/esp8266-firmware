#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "http.h"

// TODO: refacoring

ferr_t do_http_request(const char *host, int port, const char *method,
                       const char *path, const void *headers, const void *payload,
                       size_t payload_size, uint8_t **buf, size_t *buf_size,
                       int *offset, bool tls) {

    if (*buf_size == 0)
        return BERR_NOMEM;

    Serial.print((tls) ? "https: " : "http: ");
    if (offset) {
        Serial.print(" offset=");
        Serial.print(*offset);
    }
    Serial.print(method);
    Serial.print(" ");
    Serial.print(host);
    Serial.print(":");
    Serial.print(port, DEC);
    Serial.println(path);

    WiFiClient *client;
    if (tls) {
        client = new WiFiClientSecure();
    } else {
        client = new WiFiClient();
    }

    ESP.wdtFeed();
    if (!client->connect(host, port)) {
        Serial.println("error: failed to connect");
        return BERR_CONNECT;
    }

    client->print(String(method) + " " + path + " HTTP/1.0\r\n" +
                  "Host: " + host + "\r\n" +
                  "Connection: close\r\n");

    if (payload_size > 0)
        client->print("Content-Length:" + String(payload_size) + "\r\n");

    int offset_tmp = 0;
    uintptr_t offset_end;
    if (offset) {
        offset_end = *offset + *buf_size - 1;
        client->print("Range: bytes=" + String(*offset) + "-" + String(offset_end) + "\r\n");
    } else {
        offset_end = *buf_size - 1;
        offset = &offset_tmp;
    }


    client->print((const char *) headers);
    client->print("\r\n");
    client->write((const char *) payload, payload_size);

    while (!client->available()) {
        // connecting
        // TODO: implement timeout
    }

    ESP.wdtFeed();

    size_t content_length = 0;
    while (client->available() || client->connected()) {
        // connected
        // TODO: support Content-Length
        // TODO: support redirection
        String l = client->readStringUntil('\n');
        if (l.equals("\r"))
            break;

        if (l.startsWith("Content-Length:")) {
            content_length = atol(l.c_str() + l.indexOf(' ') + 1);
        }
    }

    while ((client->available() || client->connected()) && content_length > 0) {
        size_t num;
        while ((num = client->read(*buf, *buf_size)) > 0) {
            *offset += num;
            if (offset_end > (uintptr_t) *offset && *buf_size == num) {
                Serial.println("http: buffer is too short");
                return BERR_NOMEM;
            }

            *buf += num;
            *buf_size -= num;
            content_length -= num;
        }
    }

    ESP.wdtFeed();
    return BERR_OK;
}


ferr_t http_request(const char *host, int port, const char *method,
                    const char *path, const void *headers, const void *payload,
                    size_t payload_size, void *buf, size_t buf_size, bool tls) {

    ferr_t r = do_http_request(host, port, method, path, headers,
                               payload, payload_size,
                              (uint8_t **) &buf, &buf_size,
                               nullptr, tls);

    return r;
}
