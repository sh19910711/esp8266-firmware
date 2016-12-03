#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "update.h"
#include "consts.h"
#include "i2c.h"
#include "interface.h"


const char *SERVER_HOST     = "__VERY_VERY_LONG_SERVER_HOST_NAME__REPLACE_ME__";
const char *SERVER_PORT_STR = "__PORT__REPLACE_ME__";
const char *SERVER_TLS_STR  = "__TLS__REPLACE_ME__";
const char *WIFI_SSID      = "__WIFI_SSID__REPLACE_ME__";
const char *WIFI_USERNAME  = "__WIFI_USERNAME__REPLACE_ME__";
const char *WIFI_PASSWORD  = "__WIFI_PASSWORD__REPLACE_ME__";
const char *WIFI_TLS_CERT  = "__WIFI_TLS_CERT__REPLACE_ME__";
const char *WIFI_TLS_KEY   = "__WIFI_TLS_KEY__REPLACE_ME__";
const char *DEVICE_SECRET  = "__VERY_VERY_LONG_DEVICE_SECRET__REPLACE_ME__";
int SERVER_PORT;
bool SERVER_TLS;
String SERVER_URL;


static void connect_to_wifi() {
    Serial.print("firmware: connecting to ");
    Serial.println(WIFI_SSID);
    if (*WIFI_TLS_CERT) {
        Serial.println("wifi: enable eap-tls");
    }

    WiFi.mode(WIFI_STA);
    if (*WIFI_TLS_CERT) {
        wifi_enable_eap_tls(WIFI_TLS_CERT, WIFI_TLS_KEY);
    }
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("");
    Serial.println("firmware: connected");
    Serial.print("firmware: my IP address is ");
    Serial.println(WiFi.localIP());
}

static void hello() {
  const uint8 setup[] = {
      0xAE, //display off
      0xA4, //RAM reset
      0xA5, //Entire display ON
      0x00, //set lower column address
      0x10, //set higher column address
      0x00, //set display start line
      0x2E, //Deactivate scrollスクロール表示解除
      0x21, //set Column Address
      0x00, //Column Start Address←水平開始位置はここで決める(0～126)
      B01111111, //Column Stop Address　画面をフルに使う
      0x22, //Set Page Address
      0x00, //start page address
      0x07, //stop page address
      0xB0, //set page start address←垂直開始位置はここで決める8bitで１ページ(B0～B7)
      0x81, //contrast control コントラスト設定オン
      0x7f, //127　コントラスト0-127
      0xA7, //normal / reverse A7ならば白黒反転
      0xC0, //Com scan direction←開始位置を右下から始める
      0x8d, //set charge pump enableチャージポンプを入れないと表示されない
      0x14, //charge pump ON
      0xAF, //display ON
  };
  i2c_send(0x3c, setup, sizeof(setup));

  //Clear Display ディスプレイを黒で塗りつぶす
  int i, j;
  for(i=0; i<8; i++){
    const uint8 fill1[] = {
        0x00, //set display start line
        0xB0 | i, //set page start address
        0x21, //set column addres
        0x00, //start column addres←水平開始位置はここで決める(0～126)
        B01111111, //stop column addres 画面をフルに使う
    };
    i2c_send(0x3c, fill1, sizeof(fill1));
    for(j=0; j<16; j++){
      const unsigned char fill2[9] = {0x40};
      memset((void *)(fill2+1), B00111100, 8);
      i2c_send(0x3c, fill2, sizeof(fill2));
    }
  }
}

extern "C" void boot() {
    Serial.begin(115200);
    Serial.println();

    Serial.println("firmware: Hello!");

    // TODO: we don't need this, probably
    Serial.println("firmware: activating IRAM from 0x40108000");
    *((uint32_t *) 0x3ff00024) |= 0x10;

    Serial.println("firmware: initializing I2C...");
    i2c_init();
    hello();

    Serial.println("firmware: initializing SERVER_* constants...");
    SERVER_TLS   = !(strcmp(SERVER_TLS_STR, "no") == 0);
    SERVER_PORT  = atoi(SERVER_PORT_STR);
    SERVER_URL  += (SERVER_TLS)? "https://" : "http://";
    SERVER_URL  += SERVER_HOST;
    SERVER_URL  += ":";
    SERVER_URL  += SERVER_PORT;

    Serial.print("firmware: server is ");
    Serial.println(SERVER_URL);

    connect_to_wifi();

    Serial.println("firmware: downloading an app...");
    send_first_heartbeat();
}


void loop() {
}
