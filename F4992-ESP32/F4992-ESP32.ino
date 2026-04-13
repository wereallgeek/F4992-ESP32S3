/*
 This code is based on a template that was made by Neodyme under the MIT license. Thanks Neodyme!
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
uint16_t logLabelId; 

//Turntable config========
uint16_t device_name_text, highVerbosity_switch;
String stored_devicename;
bool highVerbosity;
bool firstPassCompleted = false;
//Turntable config========

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
const char* hostname = "F4992-ESP32-S3";
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

bool softSpeedInverter = false;

const char* espuiIndicatorElementStyle = " !important; width: 15px !important; height: 15px !important; border-radius: 50% !important; display: inline-block !important; vertical-align: middle !important; margin: 5px !important; border: 1px solid #222 !important;";
const char* btnStyle = "background-color: #000 !important; color: #fff !important; font-size: 10px !important; min-width: 55px !important; height: 25px !important; padding: 0 5px !important; line-height: 25px !important; border: 1px solid #444 !important; display: inline-block !important; vertical-align: middle !important; text-align: center !important;";
const char* swStyleOFF = "width: 40px !important; height: 22px !important; background-color: #000 !important; border: 1px solid #444 !important; border-radius: 11px !important; display: inline-block !important; vertical-align: top !important; transform: scale(0.8) !important; margin-top: -3px !important; margin-left: -5px !important; margin-right: -5px !important;";
const char* swStyleON  = "width: 40px !important; height: 22px !important; background-color: #2ECC71 !important; border: 1px solid #27AE60 !important; border-radius: 11px !important; display: inline-block !important; vertical-align: top !important; transform: scale(0.8) !important; margin-top: -3px !important; margin-left: -5px !important; margin-right: -5px !important;";

//ESPUI turntable GUI fields
uint16_t armStatusLabelId, armPositionLabelId, ledId;

//ESPUI==================================================================================================================


//SETUP=========================
void setup() {
  firstPassCompleted = false;
  Serial.begin(115200);
  Serial.println();

  //Custom setup...............
  simpleOTAsetup();
  turntableSetup();
  //Custom setup...............

  //pinMode(LED_BUILTIN, OUTPUT);
  preferences.begin("Settings");
  wifi_init();
  espui_init();

  simpleOTAbegin();

  firstPassCompleted = true;
  webSerialPrintln(String(millis()) + " - [" + stored_devicename + "] awoken");
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

  turntableLoop();
}
//LOOP==========================================
