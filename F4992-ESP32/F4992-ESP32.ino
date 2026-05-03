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
uint16_t statusLabelId, serialLabelId;
String option;
String stored_ssid, stored_pass;
String stored_mqtt_server, stored_mqtt_user, stored_mqtt_pass, stored_mqtt_topic_in, stored_mqtt_topic_out;
bool mqtt_enabled = false;

bool softSpeedInverter = false;

const char* espuiMainpageStyle =  "<style>body{background-color:black;color:white!important;}input{background-color:#333!important;color:white!important;border:1px solid #444!important;border-radius:4px!important;outline:none!important;box-sizing:border-box!important;padding:0 5px!important;}input:focus{background-color:#444!important;}</style>Controls";
const char* espuiStatusStyle =    " !important; width: 150px !important; height: 32px !important; border-radius: 4px !important; display: inline-block !important; border: 1px solid #444 !important; font-family: 'Segoe UI',Roboto,sans-serif !important; font-size: 0.85rem !important; line-height: 32px !important; text-align: center !important; vertical-align: middle !important; margin-top: 20px !important;";
const char* espuiNumberStyle =    " !important; width: 75px !important; height: 32px !important; border-radius: 4px !important; display: inline-block !important; border: 1px solid #444 !important; font-family: 'Segoe UI',Roboto,sans-serif !important; font-size: 0.85rem !important; line-height: 32px !important; text-align: center !important; vertical-align: middle !important; margin-top: 20px !important;";
const char* espuiIconStyle =      "color: white !important; width: 41px !important; height: 32px !important; background-color: transparent !important; border: none !important; box-shadow: none !important; display: inline-block !important; font-family: 'Segoe UI',Roboto,sans-serif !important; font-size: 2rem !important; line-height: 32px !important; text-align: right !important; vertical-align: middle !important; padding-right: 10px !important; margin-top: 20px !important;";
const char* espuiLabelStyle =     "color:white!important;width:60px!important;height:32px!important;display:inline-block!important;text-align:right!important;vertical-align:middle!important;padding-right:10px!important;margin-top:20px!important;line-height:32px!important;box-sizing:border-box!important;background-color:transparent!important;border:none!important;box-shadow:none!important;";
const char* espuiLongLabelStyle = "color:white!important;width:45%!important;height:32px!important;display:inline-block!important;text-align:right!important;vertical-align:middle!important;padding-right:10px!important;margin-top:20px!important;line-height:32px!important;box-sizing:border-box!important;background-color:transparent!important;border:none!important;box-shadow:none!important;";
const char* espuiTextLabelStyle = "color:white!important;width:25%!important;height:32px!important;display:inline-block!important;text-align:right!important;vertical-align:middle!important;padding-right:10px!important;margin-top:20px!important;line-height:32px!important;box-sizing:border-box!important;background-color:transparent!important;border:none!important;box-shadow:none!important;";
const char* espuiSwtLabelStyle =  "color: white !important; width: 45% !important; height: 32px !important; background-color: transparent !important; border: none !important; box-shadow: none !important; display: inline-block !important; font-family: 'Segoe UI',Roboto,sans-serif !important; font-size: 0.25rem !important; line-height: 34px !important; text-align: right !important; vertical-align: top !important; padding-right: 10px !important; margin-top: 22px !important;";
const char* espuiIndElemStyle =   " !important; width: 15px !important; height: 15px !important; border-radius: 50% !important; display: inline-block !important; vertical-align: middle !important; margin: 5px !important; border: 1px solid #222 !important;";
const char* btnStyle =            "background-color: #000 !important; color: #fff !important; font-size: 10px !important; min-width: 55px !important; height: 25px !important; padding: 0 5px !important; line-height: 25px !important; border: 1px solid #444 !important; display: inline-block !important; vertical-align: middle !important; text-align: center !important;";
const char* swStyleOFF =          "width: 40px !important; height: 22px !important; background-color: #000 !important; border: 1px solid #444 !important; border-radius: 11px !important; display: inline-block !important; vertical-align: top !important; transform: scale(0.8) !important; margin-top: -3px !important; margin-left: -5px !important; margin-right: -5px !important;";
const char* swStyleON  =          "width: 40px !important; height: 22px !important; background-color: #2ECC71 !important; border: 1px solid #27AE60 !important; border-radius: 11px !important; display: inline-block !important; vertical-align: top !important; transform: scale(0.8) !important; margin-top: -3px !important; margin-left: -5px !important; margin-right: -5px !important;";
const char* commandConsoleStyle = "text-align: left; font-family: 'Courier New', monospace; white-space: pre; font-size: 10px;";
const char* commandInputStyle =   "color: #000000 !important; background-color: #ffffff !important; text-align: left; font-family: 'Courier New', monospace; white-space: pre; font-size: 14px; border: 1px solid #444 !important;";
const char* espuiTelStyle =       "width:75px!important;height:32px!important;display:inline-block!important;vertical-align:top!important;margin-top:20px!important;text-align:center!important;box-sizing:border-box!important;";
const char* espuiTextSetupStyle = "width:70%!important;height:32px!important;display:inline-block!important;vertical-align:middle!important;margin-top:20px!important;box-sizing:border-box!important;padding-left:10px!important;";

const char* record33style =       "width:80px;height:80px;border-radius:50%;background:radial-gradient(circle,#000 7%,transparent 8%),radial-gradient(circle,#555 18%,#111 19%,#111 100%);border:4px double #222;"
                                  "box-shadow:inset 0 0 10px #000,0 4px 10px rgba(0,0,0,0.5);color:rgba(255,255,255,0.8);line-height:80px;text-align:center;font-weight:bold;display:inline-block;vertical-align:middle;font-size:3.2rem;margin-left:20px !important;";

const char* record45style =       "width:80px;height:80px;border-radius:50%;background:radial-gradient(circle,#000 25%,transparent 26%),radial-gradient(circle,#555 45%,#111 46%,#111 100%);border:4px solid #222;"
                                  "box-shadow:inset 0 0 8px #000,0 3px 8px rgba(0,0,0,0.5);color:rgba(255,255,255,0.8);line-height:80px;text-align:center;font-weight:bold;display:inline-block;vertical-align:middle;font-size:3.2rem;margin-left:20px !important;";

const char* recordNodiscStyle =   "width:80px;height:80px;border-radius:50%;background:radial-gradient(circle,#000 4%,transparent 5%),radial-gradient(circle,#777 5%,#333 6%,#333 100%);border:2px solid #444;"
                                  "box-shadow:inset 0 0 15px #000;color:transparent;line-height:80px;text-align:center;font-weight:bold;display:inline-block;vertical-align:middle;font-size:3.2rem;margin-left:20px !important;";



//ESPUI turntable GUI fields
uint16_t armStatusLabelId, armPositionLabelId, repeatId, ledId, recordsizeLabelId, lifterStatusId, dcmStatusId;
uint16_t detectionDurationLabelId, muteDurationLabelId, irCycleDurationLabelId, irTresholdLabelId, armPresetValue30LabelId, armPresetValue17LabelId, armPresetValueEndLabelId;
//ESPUI==================================================================================================================


enum actionTypeForStats {MANUAL, CONSOLE, WEB, MQTT, AUTO};

volatile bool uiPressRepeat    = false;
volatile bool uiPressMoveIn    = false;
volatile bool uiPressMoveOut   = false;
volatile bool uiPressUpDown    = false;
volatile bool uiPressStartStop = false;
volatile int  uiTypeStartStop  = AUTO;
volatile bool uiInvert         = false;
volatile bool uiAskMoveHome    = false;
volatile bool uiAskMoveEnd     = false;
volatile bool uiAskMove30      = false;
volatile bool uiAskMove17      = false;
volatile bool uiAskMoveNot     = false;
volatile bool uiRequestInit    = false;
volatile bool uiAskReport      = false;
volatile bool uiAskInfra       = false;
  
TaskHandle_t Handle_Turntable;

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
  espui_init();

  simpleOTAbegin();

  firstPassCompleted = true;
  xTaskCreatePinnedToCore(TaskTurntable, "TurntableTask", 10000, NULL, 1, &Handle_Turntable, 1);

  webSerialPrintln(String("FW ver ") + firmwareVersion());
  webSerialPrintln(String("[") + stored_devicename + "] awoken");
}
//SETUP=========================

//LOOP==========================================
void loop() {
  if (!wificonnected) dnsServer.processNextRequest();
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    SerialCommand(input);
  }
  mqtt_loop();

  //update GUI
  turntableUiUpdate();

  vTaskDelay(1); 
}
//core 1 - handles everything but ESPUI
void TaskTurntable(void * pvParameters) {
  for(;;) {
    turntableLoop();
    vTaskDelay(1);
  }
}
//LOOP==========================================
