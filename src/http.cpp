#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "http.h"

// TODO: refacoring

ferr_t do_http_request(const char *host, int port, const char *method,
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
        Serial.println("error: failed to connect");
        return BERR_CONNECT;
    }

    client.print(String(method) + " " + path + " HTTP/1.0\r\n" +
                 "Host: " + host + "\r\n" +
                 "Range: bytes=" + String(*offset) + "-" + String(*offset + *buf_size) + "\r\n" +
                 "Connection: close\r\n" +
                 "\r\n");

    // TODO: support headers and payload

    while (!client.available()) {
        // connecting
        // TODO: implement timeout
    }

    ESP.wdtFeed();

    bool in_header = true;
    bool is_eof = false;
    while (client.available() || client.connected()) {
        // connected
        // TODO: support Content-Length
        // TODO: support redirection
        if (in_header) {
            String l = client.readStringUntil('\n');
            if (l.startsWith("X-End-Of-File:")) {
                is_eof = true;
            }

            if (l.equals("\r")) {
                in_header = false;
            }
        } else {
            size_t num;
            while ((num = client.read(*buf, *buf_size)) > 0) {
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
    return (is_eof) ? BERR_EOF : BERR_OK;
}


ferr_t http_request(const char *host, int port, const char *method,
                    const char *path, const void *headers, const void *payload,
                    size_t payload_size, void *buf, size_t buf_size) {

    int offset = 0;
    ferr_t r = do_http_request(host, port, method, path, headers,
                               payload, payload_size,
                              (uint8_t **) &buf, &buf_size,
                               &offset);

    return r;
}


ferr_t xeof_http_request(const char *host, int port, const char *method,
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
