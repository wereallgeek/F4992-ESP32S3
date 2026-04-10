/*
 This code is based on a template been made by Neodyme under the MIT license. Thanks Neodyme!
 Youtube : https://www.youtube.com/neodymetv
 Twitch : https://www.twitch.tv/ioodyme
 Github : https://github.com/n3odym3
 ESPUI documentation : https://github.com/s00500/ESPUI#readme
*/
//Let's build geeky things
/*
 This project is meant to function as a replacement F-4992 microcomputer board
  for a  linear tracking turntable model P-L45 or P-L55 (and perhaps others)
*/


//OTA===================================
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Update.h>
AsyncWebServer updateserver(4992);


// HTML
const char* updateIndex = 
"<form method='POST' action='/doupdate' enctype='multipart/form-data'>"
  "<h3>Let's build geeky things</h2>"
  "<p></p>"
  "<h2>Firmware update</h3>"
  "<p></p>"
  "<input type='file' name='update' accept='.bin'>"
  "<input type='submit' value='UPDATE'>"
"</form>";
//OTA===================================

//Web server==================================
#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined ESP32
#include <WiFi.h>
#define LED_BUILTIN 2
#else
#error Architecture unrecognized by this code.
#endif

#include <Preferences.h>
Preferences preferences;
#include <DNSServer.h>
#define DNS_PORT 53
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
const char* hostname = "ESPUI-Demo";
bool wificonnected = false;
//Web server==================================

//MQTT=========================
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);
#define mqtt_retry_delay 10000
unsigned long last_millis = 0;
//MQTT=========================

//ESPUI=================================================================================================================
#include <ESPUI.h>
uint16_t wifi_ssid_text, wifi_pass_text;
uint16_t mqtt_server_text, mqtt_topic_in_text, mqtt_topic_out_text, mqtt_user_text, mqtt_pass_text, mqtt_enabled_switch;
uint16_t statusLabelId, serialLabelId;
String option;
String stored_ssid, stored_pass;
String stored_mqtt_server, stored_mqtt_user, stored_mqtt_pass, stored_mqtt_topic_in, stored_mqtt_topic_out;
bool mqtt_enabled = false;
//ESPUI==================================================================================================================

//Custom libraries..............
void setupOTA(AsyncWebServer &otaServer, const char* path) {
    //OTA update  
    // 1. form
    otaServer.on(path, HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/html", updateIndex);
    });
    // 2. ending upload and reboot
    otaServer.on("/doupdate", HTTP_POST, [](AsyncWebServerRequest *request){
      bool failure = Update.hasError();
      request->send(200, "text/plain", failure ? "Error" : "SUCCES : Reboot...");
      delay(1000);
      if(!failure) ESP.restart();
    }, 
    // 3. manage .bin
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
      if (!index) { // 1st pack
        Serial.printf("Update start: %s\n", filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { 
          Update.printError(Serial);
        }
      }
      if (len) { // write each part in flash
        if (Update.write(data, len) != len) {
          Update.printError(Serial);
        }
      }
      if (final) { // done. validate
        if (Update.end(true)) {
          Serial.printf("Update end: %u bytes\n", index + len);
        } else {
          Update.printError(Serial);
        }
      }
    });
}
//Custom libraries..............

//SETUP=========================
void setup() {

  Serial.begin(115200);
  Serial.println();

  //Custom setup...............
  setupOTA(updateserver, "/");
  //Custom setup...............

  //pinMode(LED_BUILTIN, OUTPUT);
  preferences.begin("Settings");
  wifi_init();
  espui_init();

  updateserver.begin();
}
//SETUP=========================

//LOOP==========================================
void loop() {
  dnsServer.processNextRequest();
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    ESPUI.print(serialLabelId, input);
    SerialSetup(input);
  }
  mqtt_loop();

  //Custom loop.................................
  //Your code HERE !
  //Custom loop.................................
}
//LOOP==========================================
