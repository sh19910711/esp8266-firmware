#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "update.h"
#include "consts.h"

const char *CODESTAND_HOST = "__CODESTAND_MGMT_SERVER_HOSTNAME__REPLACE_ME__";
const char *WIFI_SSID      = "__WIFI_SSID__REPLACE_ME__";
const char *WIFI_PASSWORD  = "__WIFI_PASSWORD__REPLACE_ME__";
const char *DEVICE_SECRET  = "__VERY_VERY_LONG_DEVICE_SECRET__REPLACE_ME__";

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

    Serial.println("firmware: activating IRAM from 0x40108000");
    *((uint32_t *) 0x3ff00024) |= 0x10;

    Serial.println("firmware: downloading an app");
    update_status();
}
