#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "update.h"
#include "consts.h"

const char *SERVER_HOST     = "__VERY_VERY_LONG_SERVER_HOST_NAME__REPLACE_ME__";
const char *SERVER_PORT_STR = "__PORT__REPLACE_ME__";
const char *SERVER_TLS_STR  = "__TLS__REPLACE_ME__";
const char *WIFI_SSID      = "__WIFI_SSID__REPLACE_ME__";
const char *WIFI_PASSWORD  = "__WIFI_PASSWORD__REPLACE_ME__";
const char *DEVICE_SECRET  = "__VERY_VERY_LONG_DEVICE_SECRET__REPLACE_ME__";
int SERVER_PORT;
bool SERVER_TLS;
String SERVER_URL;

extern "C" void boot() {
    Serial.begin(115200);
    Serial.println();

    Serial.println("I am a Codestand firmware for ESP8266!");
    Serial.print("firmware: connecting to ");
    Serial.println(WIFI_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("firmware: connected");
    Serial.print("firmware: my IP address is ");
    Serial.println(WiFi.localIP());

    // TODO: we don't need this, probably
    Serial.println("firmware: activating IRAM from 0x40108000");
    *((uint32_t *) 0x3ff00024) |= 0x10;

    SERVER_TLS  = !(strcmp(SERVER_TLS_STR, "no") == 0);
    SERVER_PORT = atoi(SERVER_PORT_STR);
     SERVER_URL += (SERVER_TLS)? "https://" : "http://";
    SERVER_URL += SERVER_HOST;
    SERVER_URL += ":";
    SERVER_URL += SERVER_PORT;

    Serial.print("firmware: server is ");
    Serial.println(SERVER_URL);

    Serial.println("firmware: downloading an app");
    send_first_heartbeat();
}
