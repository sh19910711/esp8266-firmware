#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "http.h"

// TODO: refacoring

int do_http_request(const char *method, const char *host, int port,
                    const char *path, const void *headers, size_t headers_size,
                    const void *payload, size_t payload_size,
                    uint8_t **buf, size_t *buf_size, size_t *resp_size,
                    int *offset, bool tls) {

    if (*buf_size == 0)
        return 0;

    Serial.print((tls) ? "https: " : "http: ");
    Serial.print(method);
    Serial.print(" ");
    Serial.print(host);
    Serial.print(":");
    Serial.print(port, DEC);
    Serial.print(path);

    if (offset) {
        Serial.print(" offset=");
        Serial.print(*offset);
    }

    Serial.println("");

    WiFiClient *client;
    if (tls) {
        client = new WiFiClientSecure();
    } else {
        client = new WiFiClient();
    }

    ESP.wdtFeed();
    if (!client->connect(host, port)) {
        Serial.println("error: failed to connect");
        return 0;
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


    client->write((const char *) headers, headers_size);
    client->print("\r\n");
    client->write((const char *) payload, payload_size);

    while (!client->available()) {
        // connecting
        // TODO: implement timeout
    }

    ESP.wdtFeed();

    int status_code = 200; // TODO
    size_t content_length = 0;
    while (client->available() || client->connected()) {
        // connected
        // TODO: support Content-Length
        // TODO: support redirection
        String l = client->readStringUntil('\n');
        if (l.equals("\r"))
            break;

        if (l.startsWithIgnoreCase("Content-Length:")) {
            content_length = atol(l.c_str() + l.indexOf(' ') + 1);
        }
    }

    *resp_size = 0;
    if (*buf) {
        while ((client->available() || client->connected()) && content_length > 0) {
            size_t num;
            while ((num = client->read(*buf, *buf_size)) > 0) {
                *offset += num;
                if (offset_end > (uintptr_t) *offset && *buf_size == num) {
                    Serial.println("http: buffer is too short");
                    return 0;
                }

                *resp_size += num;
                *buf += num;
                *buf_size -= num;
                content_length -= num;
            }
        }
    }

    ESP.wdtFeed();
    return status_code;
}


int _http_request(const char *method, const char *url, size_t url_size,
                  const void *headers, size_t headers_size,
                  const void *payload, size_t payload_size,
                  void *buf, size_t buf_size, size_t *resp_size) {

    int port;
    bool tls;
    const char *rest;
    if (!strncmp(url, "http://", 7)) {
        port = 80;
        tls = false;
        rest = url + 7;
    } else if (!strncmp(url, "https://", 8)) {
        port = 443;
        tls  = true;
        rest = url + 8;
    } else {
        Serial.println("http: unsupported scheme");
        return 0;
    }

    String host;
    while (url_size && *rest != ':' && *rest != '/') {
        host += *rest;
        rest++;
        url_size--;
    }

    if (*rest == ':') {
        String s;
        rest++; // skip ':'
        while (url_size && *rest != '/') {
            s += *rest;
            rest++;
            url_size--;
        }

        port = atol(s.c_str());
    }

    while (url_size && *rest != ':' && *rest != '/') {
        host += *rest;
        rest++;
        url_size--;
    }

    if (*rest == ':') {
        String s;
        rest++; // skip ':'
        while (url_size && *rest != '/') {
            s += *rest;
            rest++;
            url_size--;
        }

        port = atol(s.c_str());
    }

    String path;
    while (url_size) {
        path += *rest;
        rest++;
        url_size--;
    }

    if (path.length() == 0)
        path += '/';

    return do_http_request(method, host.c_str(), port, path.c_str(),
                           headers, headers_size, payload, payload_size,
                           (uint8_t **) &buf, &buf_size, resp_size,
                           nullptr, tls);
}


int http_request(const char *method, String url, String headers,
                 const void *payload, size_t payload_size,
                 void *buf, size_t buf_size, size_t *resp_size) {

    return _http_request(method, url.c_str(), url.length(),
                         headers.c_str(), headers.length(),
                         payload, payload_size,
                         buf, buf_size, resp_size);
}
