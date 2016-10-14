#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "update.h"
#include "consts.h"


const char *device_secret = DEVICE_SECRET;
const char *wifi_ssid   = WIFI_SSID;
const char *wifi_password = WIFI_PASSWORD;

extern "C" void boot() {
    Serial.begin(115200);
    Serial.println();

    Serial.println("I am a Codestand firmware for ESP8266!");
    Serial.print("firmware: connecting to ");
    Serial.println(wifi_ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_ssid, wifi_password);
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
