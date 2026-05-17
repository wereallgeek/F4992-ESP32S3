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
  for a linear tracking turntable model P-L35, P-L45 or P-L55 (and perhaps others)
*/
volatile int taskDelay = 1;
uint16_t logLabelId; 

//Turntable config========
uint16_t device_name_text, highVerbosity_switch;
String stored_devicename;
volatile bool highVerbosity = false;
volatile bool firstPassCompleted = false;
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
Preferences settings;
#include <DNSServer.h>
#include <ESPmDNS.h>
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
volatile bool otaupdateInProgress = false;
String option;
String stored_ssid, stored_pass;
String stored_mqtt_server, stored_mqtt_user, stored_mqtt_pass, stored_mqtt_topic_in, stored_mqtt_topic_out;
bool mqtt_enabled = false;

volatile bool softSpeedInverter = false;

//ESPUI turntable GUI fields
uint16_t armStatusLabelId, armPositionLabelId, repeatId, ledId, recordsizeLabelId, lifterStatusId, dcmStatusId, spd_switch, firmwareUpdate;
uint16_t detectionDurationLabelId, muteDurationLabelId, timeoutLabelId, timeoutEnabledLabelId, ffwdRewSkipAmmountLabelId, irCycleDurationLabelId;
uint16_t irTresholdLabelId, armPresetValue30LabelId, armPresetValue17LabelId, armPresetValueEndLabelId, serialLabelId;
//ESPUI==================================================================================================================


enum actionTypeForStats {MANUAL, CONSOLE, WEB, MQTT, AUTO};

volatile bool uiPressRepeat    = false;
volatile bool uiPressMoveIn    = false;
volatile bool uiPressMoveOut   = false;
volatile bool uiPressUpDown    = false;
volatile bool uiPressStartStop = false;
volatile bool uiPressJustStart = false;
volatile bool uiPressJustStop  = false;
volatile int  uiTypeStartStop  = AUTO;
volatile bool uiInvert         = false;
volatile bool uiAskMoveHome    = false;
volatile bool uiAskMoveEnd     = false;
volatile bool uiAskMove30      = false;
volatile bool uiAskMove17      = false;
volatile bool uiAskMoveNot     = false;
volatile bool uiAskMoveTo      = false;
volatile int  uiToPosition     = 0;
volatile bool uiRequestInit    = false;
volatile bool uiAskReport      = false;
volatile bool uiAskInfra       = false;
volatile bool uiAskfwupdate    = false;

TaskHandle_t Handle_Turntable;

volatile bool UserWebBypassRequest = false;
volatile bool UserTTBypassRequest  = false;

//SETUP=========================
void setup() {
  firstPassCompleted = false;
  cpuInfo_setup();
  Serial.begin(115200);
  Serial.println();

  analogSetAttenuation(ADC_11db);
  
  settings.begin("Settings");
  //Custom setup...............
  turntableSetup();
  simpleOTAsetup();

 
  wifi_init();
  if (UserWebBypassRequest) emergencyServerSetup();
  else espui_init();

  simpleOTAbegin();

  firstPassCompleted = true;
  if (!UserTTBypassRequest) xTaskCreatePinnedToCore(TaskTurntable, "TurntableTask", 10000, NULL, 3, &Handle_Turntable, 1);

  webSerialPrintln(String("FW ver ") + firmwareVersion());
  webSerialPrintln(String("[") + stored_devicename + "] awoken");
}
//SETUP=========================

//LOOP==========================================
void loop() {
  if (!otaupdateInProgress) {
    if (!wificonnected) dnsServer.processNextRequest();
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      SerialCommand(input);
    }
    mqtt_loop();

    //update GUI
    if (!UserTTBypassRequest) turntableUiUpdate();
  }

  vTaskDelay(taskDelay); 
}
//core 1 - handles everything but ESPUI
void TaskTurntable(void * pvParameters) {
  for(;;) {
    if (!otaupdateInProgress) turntableLoop();
    vTaskDelay(taskDelay);
  }
}
//LOOP==========================================
