//================================================
// ESPUI 
//
// configuration of the ESPUI interface
//
//================================================

const char* espuiMainpageStyle =  "<style>body{background-color:black;color:white!important;}input{background-color:#333!important;color:white!important;border:1px solid #444!important;border-radius:4px!important;outline:none!important;box-sizing:border-box!important;padding:0 5px!important;}input:focus{background-color:#444!important;}</style>Controls";

const char* espuiIconStyle      = "color:#fff;width:41px;height:32px;background:0 0;border:0;box-shadow:none;display:inline-block;font:2rem/32px sans-serif;text-align:right;vertical-align:middle;padding-right:10px;margin-top:20px!important;";
const char* espuiLabelStyle     = "color:#fff;width:60px;height:32px;display:inline-block;text-align:right;vertical-align:middle;padding-right:10px;margin-top:20px;line-height:32px;box-sizing:border-box;background:0 0;border:0;box-shadow:none!important;";
const char* espuiTextLabelStyle = "color:#fff;width:25%;height:32px;display:inline-block;text-align:right;vertical-align:middle;padding-right:10px;margin-top:20px;line-height:32px;box-sizing:border-box;background:0 0;border:0;box-shadow:none!important;";
const char* espuiLongLabelStyle = "color:#fff;width:45%;height:32px;display:inline-block;text-align:right;vertical-align:middle;padding-right:10px;margin-top:20px;line-height:32px;box-sizing:border-box;background:0 0;border:0;box-shadow:none!important;";
const char* espuiLTxtLabelStyle = "color:#fff;width:65%;height:32px;display:inline-block;text-align:right;vertical-align:middle;padding-right:10px;margin-top:20px;line-height:32px;box-sizing:border-box;background:0 0;border:0;box-shadow:none!important;";
const char* espuiSwtLabelStyle  = "color:#fff!important;width:45%!important;height:32px!important;background:0 0!important;border:none!important;box-shadow:none!important;display:inline-block!important;font-family:'Segoe UI',Roboto,sans-serif!important;font-size:0.85rem!important;line-height:34px!important;text-align:right!important;vertical-align:top!important;padding-right:10px!important;margin-top:22px!important;";
const char* espuiLSwtLabelStyle = "color:#fff;width:65%;height:32px;display:inline-block;text-align:right;vertical-align:middle;padding-right:10px;margin-top:7px!important;line-height:20px!important;box-sizing:border-box;background:0 0;border:0;box-shadow:none!important;";
const char* btnStyle            = "background:#000;color:#fff;font:10px/25px sans-serif;min-width:55px;height:25px;padding:0 5px;border:1px solid #444;display:inline-block;vertical-align:middle;text-align:center!important;";
const char* espuiTelStyle       = "width:75px;height:32px;display:inline-block;vertical-align:top;margin-top:20px;text-align:center;box-sizing:border-box!important;";
const char* espuiTextSetupStyle = "width:70%;height:32px;display:inline-block;vertical-align:middle;margin-top:20px;box-sizing:border-box;padding-left:10px!important;";
const char* espuiStatLabel      = "background:#1e1e1e!important;color:#0f0!important;font-family:monospace!important;font-size:11px!important;width:46%!important;display:inline-block!important;margin:2px 1%!important;padding:4px!important;height:24px!important;line-height:16px!important;border:1px solid #333!important;box-sizing:border-box!important;vertical-align:top!important;";
const char* espuilargebutton    = "width: 45%; margin: 10px auto; display: block;";

//Interface conditionality
Preferences uisetup;
volatile bool espuiDebugVisible = false;
volatile bool espuiRepeatButtonVisible = true;
volatile bool espuiInvertButtonVisible = true;

void setupInterface() {
  uisetup.begin("uiSetup", false);
  loadRepeatVisibility();
  loadInvertVisibility();
}

//repeat button + indicator
bool repeatbuttonVisible() {
  return espuiRepeatButtonVisible;
}

void loadRepeatVisibility() {
  espuiRepeatButtonVisible = settings.getBool("repeatbtn", true);
}

void writeRepeatVisibility(bool visible) {
  settings.putBool("repeatbtn", visible);
}

//Soft inverter button
bool invertbuttonVisible() {
  return espuiInvertButtonVisible;
}

void loadInvertVisibility() {
  espuiInvertButtonVisible = settings.getBool("invertbtn", true);
}

void writeInvertVisibility(bool visible) {
  settings.putBool("invertbtn", visible);
}

void reflectSwitchPosition(uint16_t id, bool state) {
  ESPUI.updateSwitcher(id, state);
  ESPUI.setElementStyle(id, getEspuiSwitchStyle(state));
}

//ESPUI=====================================================================================================================
void espui_init() {
  ESPUI.setVerbosity(Verbosity::Quiet);//todo: link to settings
  setupInterface();

  //create tabs here to get urls with #1, #2, etc.  
  auto tonearmtab = ESPUI.addControl(Tab, "", "Status");
  auto addonstab = ESPUI.addControl(Tab, "", "Addons");
  auto configtab = ESPUI.addControl(Tab, "", "Config");
  auto wifitab = ESPUI.addControl(Tab, "", "WiFi");
    
  //Addons - configuration for ui interface, features, and hardware addons --------------------------------------------------------

  //--ui config block
  auto rptswitchlabel = ESPUI.addControl(Label, "UI Config (requires Reboot)", "Repeat feature : ", None, addonstab, noCallback);
  ESPUI.setElementStyle(rptswitchlabel, espuiLSwtLabelStyle);  
  auto rpt_switch = ESPUI.addControl(Switcher, "", String(espuiRepeatButtonVisible), None, rptswitchlabel, switchRepeatCallback);
  ESPUI.setElementStyle(rpt_switch, getEspuiSwitchStyle(espuiRepeatButtonVisible));

  auto invrtswitchlabel = ESPUI.addControl(Label, "", "Speed inverter feature : ", None, rptswitchlabel, noCallback);
  ESPUI.setElementStyle(invrtswitchlabel, espuiLSwtLabelStyle);  
  auto invrt_switch = ESPUI.addControl(Switcher, "", String(espuiInvertButtonVisible), None, rptswitchlabel, switchInvertCallback);
  ESPUI.setElementStyle(invrt_switch, getEspuiSwitchStyle(espuiInvertButtonVisible));
  auto addonreboot = ESPUI.addControl(Button, "", "Reboot turntable", None, rptswitchlabel, ESPReset);
  ESPUI.setElementStyle(addonreboot, espuilargebutton);

  //--led strip block
  auto ledswitchlabel = ESPUI.addControl(Label, "Led strip", "Ledstrip enabled : ", None, addonstab, noCallback);
  ESPUI.setElementStyle(ledswitchlabel, espuiLSwtLabelStyle);  
  auto led_switch = ESPUI.addControl(Switcher, "", String(useLedPixel()), None, ledswitchlabel, switchLedCallback);
  ESPUI.setElementStyle(led_switch, getEspuiSwitchStyle(useLedPixel()));
  
  auto ledNbEntryLabel = ESPUI.addControl(Label, "", "Number of leds : ", None, ledswitchlabel, noCallback);
  ESPUI.setElementStyle(ledNbEntryLabel, espuiLTxtLabelStyle);  
  auto ledNbEntry = ESPUI.addControl(Text, "", String(numberOfPixels()), Dark, ledswitchlabel, numLedCallback);
  ESPUI.setElementStyle(ledNbEntry, espuiTelStyle);
  ESPUI.setInputType(ledNbEntry, "tel");

  auto ledBrightnessLabel = ESPUI.addControl(Label, "", "Brightness : ", None, ledswitchlabel, noCallback);
  ESPUI.setElementStyle(ledBrightnessLabel, espuiLTxtLabelStyle);  
  auto ledBrightness = ESPUI.addControl(Text, "", String(ledBrightnessValue()), Dark, ledswitchlabel, ledBrightnessCallback);
  ESPUI.setElementStyle(ledBrightness, espuiTelStyle);
  ESPUI.setInputType(ledBrightness, "tel");

  auto ledchaserLabel = ESPUI.addControl(Label, "", "Chaser speed : ", None, ledswitchlabel, noCallback);
  ESPUI.setElementStyle(ledchaserLabel, espuiLTxtLabelStyle);  
  auto ledchaser = ESPUI.addControl(Text, "", String(chaserSpeedValue()), Dark, ledswitchlabel, ledchaserCallback);
  ESPUI.setElementStyle(ledchaser, espuiTelStyle);
  ESPUI.setInputType(ledchaser, "tel");

  auto ledBreatherLabel = ESPUI.addControl(Label, "", "Breather adjustment : ", None, ledswitchlabel, noCallback);
  ESPUI.setElementStyle(ledBreatherLabel, espuiLTxtLabelStyle);  
  auto ledBreather = ESPUI.addControl(Text, "", String(breatherAdjValue()), Dark, ledswitchlabel, ledbreatherCallback);
  ESPUI.setElementStyle(ledBreather, espuiTelStyle);
  ESPUI.setInputType(ledBreather, "tel");
  //Addons - configuration for ui interface, features, and hardware addons --------------------------------------------------------

  //Tonearm Control - in header to be available from all tabs----------------------------------------------------------------------
  ledId = ESPUI.label(espuiMainpageStyle, Dark, "");
  ESPUI.setElementStyle(ledId, getEspuiIndicatorColor("#2c3e50"));
  auto in_button = ESPUI.addControl(Button, "", "< Left ", None, ledId, buttonInCallback);
  ESPUI.setElementStyle(in_button, btnStyle);
  auto out_button = ESPUI.addControl(Button, "", "Right >", None, ledId, buttonOutCallback);
  ESPUI.setElementStyle(out_button, btnStyle);
  auto up_button = ESPUI.addControl(Button, "", "Up/Down", None, ledId, buttonUpdownCallback);
  ESPUI.setElementStyle(up_button, btnStyle);
  auto ss_button = ESPUI.addControl(Button, "", "Start/Stop", None, ledId, buttonStartStopCallback);
  ESPUI.setElementStyle(ss_button, btnStyle);
  if (espuiRepeatButtonVisible) {
    auto rpt_button = ESPUI.addControl(Button, "", "Repeat", None, ledId, buttonRepeatCallback);
    ESPUI.setElementStyle(rpt_button, btnStyle);
    repeatId = ESPUI.addControl(Label, "", "", Dark, ledId, noCallback);
    ESPUI.setElementStyle(repeatId, getEspuiIndicatorColor("#2c3e50"));
  }
  if (espuiInvertButtonVisible) {
    spd_switch = ESPUI.addControl(Switcher, "", String(softSpeedInverter), None, ledId, buttonInvertCallback);
    ESPUI.setElementStyle(spd_switch, getEspuiSwitchStyle(false));
  }
  //Turntable Controls-------------------------------------------------------------------------------------------------------------


  //Tonearm Status-----------------------------------------------------------------------------------------------------------------
  //status
  recordsizeLabelId = ESPUI.addControl(Label, "", "status", Dark, tonearmtab, noCallback);
  ESPUI.setElementStyle(recordsizeLabelId, getEspuiDefaultRecord());
  lifterStatusId = ESPUI.addControl(Label, "", "arm", None, recordsizeLabelId, noCallback);
  ESPUI.setElementStyle(lifterStatusId, espuiIconStyle);
  armStatusLabelId = ESPUI.addControl(Label, "", "status", Dark, recordsizeLabelId, noCallback);
  ESPUI.setElementStyle(armStatusLabelId, getEspuiLabelColor("#2c3e50"));
  dcmStatusId = ESPUI.addControl(Label, "", "dcm", None, recordsizeLabelId, noCallback);
  ESPUI.setElementStyle(dcmStatusId, espuiIconStyle);
  //Tonearm Status-----------------------------------------------------------------------------------------------------------------

  //Turntable configuration--------------------------------------------------------------------------------------------------------
  auto durationLabel = ESPUI.addControl(Label, "Duration", "detection: ", None, configtab, noCallback);
  ESPUI.setElementStyle(durationLabel, espuiLongLabelStyle);  
  detectionDurationLabelId = ESPUI.addControl(Text, "", String(getDetectionDuration()), Dark, durationLabel, noCallback);
  ESPUI.setElementStyle(detectionDurationLabelId, espuiTelStyle);
  ESPUI.setInputType(detectionDurationLabelId, "tel");
  auto muteLabel = ESPUI.addControl(Label, "", "     mute: ", None, durationLabel, noCallback);
  ESPUI.setElementStyle(muteLabel, espuiLongLabelStyle);  
  muteDurationLabelId = ESPUI.addControl(Text, "", String(getMuteDuration()), Dark, durationLabel, noCallback);
  ESPUI.setElementStyle(muteDurationLabelId, espuiTelStyle);
  ESPUI.setInputType(muteDurationLabelId, "tel");

  auto infraredLabel = ESPUI.addControl(Label, "Infrared", "duration: ", None, configtab, noCallback);
  ESPUI.setElementStyle(infraredLabel, espuiLongLabelStyle);  
  irCycleDurationLabelId = ESPUI.addControl(Text, "", String(getIrCycleDuration()), Dark, infraredLabel, noCallback);
  ESPUI.setElementStyle(irCycleDurationLabelId, espuiTelStyle);
  ESPUI.setInputType(irCycleDurationLabelId, "tel");
  auto tresholdLabel = ESPUI.addControl(Label, "", "treshold: ", None, infraredLabel, noCallback);
  ESPUI.setElementStyle(tresholdLabel, espuiLongLabelStyle);  
  irTresholdLabelId = ESPUI.addControl(Text, "", String(getIrTreshold()), Dark, infraredLabel, noCallback);
  ESPUI.setElementStyle(irTresholdLabelId, espuiTelStyle);
  ESPUI.setInputType(irTresholdLabelId, "tel");

  auto presetLabel = ESPUI.addControl(Label, "Arm position presets", "12\": ", None, configtab, noCallback);
  ESPUI.setElementStyle(presetLabel, espuiLabelStyle);  
  armPresetValue30LabelId = ESPUI.addControl(Text, "Arm Presets", String(getArmPresetValue(1)), Dark, presetLabel, noCallback);
  ESPUI.setElementStyle(armPresetValue30LabelId, espuiTelStyle);
  ESPUI.setInputType(armPresetValue30LabelId, "tel");
  auto preset17Label = ESPUI.addControl(Label, "", "6\": ", None, presetLabel, noCallback);
  ESPUI.setElementStyle(preset17Label, espuiLabelStyle);  
  armPresetValue17LabelId = ESPUI.addControl(Text, "", String(getArmPresetValue(2)), Dark, presetLabel, noCallback);
  ESPUI.setElementStyle(armPresetValue17LabelId, espuiTelStyle);
  ESPUI.setInputType(armPresetValue17LabelId, "tel");
  auto presetEndLabel = ESPUI.addControl(Label, "", "End: ", None, presetLabel, noCallback);
  ESPUI.setElementStyle(presetEndLabel, espuiLabelStyle);  
  armPresetValueEndLabelId = ESPUI.addControl(Text, "", String(getArmPresetValue(3)), Dark, presetLabel, noCallback);
  ESPUI.setElementStyle(armPresetValueEndLabelId, espuiTelStyle);
  ESPUI.setInputType(armPresetValueEndLabelId, "tel");
  armPositionLabelId = ESPUI.addControl(Label, "", "position", None, presetLabel, noCallback);
  ESPUI.setElementStyle(armPositionLabelId, espuiStatLabel);
  
  auto configsave = ESPUI.addControl(Button, "Save", "Save", Peterriver, configtab, saveTurntableDetailsCallback);
  auto configApply = ESPUI.addControl(Button, "", "Apply", None, configsave, applyTurntableDetailsCallback);
  auto coinfigReset = ESPUI.addControl(Button, "", "Reset to default", None, configsave, resetTurntableDetailsCallback);
  auto firmwareandcrash = ESPUI.addControl(Label, "", String("FW v") + firmwareVersion(), None, configsave, noCallback);
  ESPUI.setElementStyle(firmwareandcrash, espuiLTxtLabelStyle);

  //Turntable configuration--------------------------------------------------------------------------------------------------------

  //WiFi---------------------------------------------------------------------------------------------------------------------------
  device_name_text = ESPUI.addControl(Text, "Device name", stored_devicename, Dark, wifitab, noCallback);
  ESPUI.setElementStyle(device_name_text, espuiTextSetupStyle);

  auto wifiLabel = ESPUI.addControl(Label, "Wi-Fi setup", "SSID: ", None, wifitab, noCallback);
  ESPUI.setElementStyle(wifiLabel, espuiTextLabelStyle);  
  wifi_ssid_text = ESPUI.addControl(Text, "SSID", stored_ssid, Dark, wifiLabel, noCallback);
  ESPUI.setElementStyle(wifi_ssid_text, espuiTextSetupStyle);
  auto passLabel = ESPUI.addControl(Label, "", "pass: ", None, wifiLabel, noCallback);
  ESPUI.setElementStyle(passLabel, espuiTextLabelStyle);  
  wifi_pass_text = ESPUI.addControl(Text, "Password", stored_pass, Dark, wifiLabel, noCallback);
  ESPUI.setElementStyle(wifi_pass_text, espuiTextSetupStyle);
  ESPUI.setInputType(wifi_pass_text, "password");
  ESPUI.addControl(Max, "", "32", None, wifi_ssid_text);
  ESPUI.addControl(Max, "", "64", None, wifi_pass_text);

  auto mqttLabel = ESPUI.addControl(Label, "MQTT setup", "IP: ", None, wifitab, noCallback);
  ESPUI.setElementStyle(mqttLabel, espuiTextLabelStyle);  
  mqtt_server_text = ESPUI.addControl(Text, "MQTT server", stored_mqtt_server, Dark, mqttLabel, noCallback);
  ESPUI.setElementStyle(mqtt_server_text, espuiTextSetupStyle);
  auto mqttuserLabel = ESPUI.addControl(Label, "", "User: ", None, mqttLabel, noCallback);
  ESPUI.setElementStyle(mqttuserLabel, espuiTextLabelStyle);  
  mqtt_user_text = ESPUI.addControl(Text, "MQTT user", stored_mqtt_user, Dark, mqttLabel, noCallback);
  ESPUI.setElementStyle(mqtt_user_text, espuiTextSetupStyle);
  auto mqttpasswordLabel = ESPUI.addControl(Label, "", "Pass: ", None, mqttLabel, noCallback);
  ESPUI.setElementStyle(mqttpasswordLabel, espuiTextLabelStyle);  
  mqtt_pass_text = ESPUI.addControl(Text, "MQTT password", stored_mqtt_pass, Dark, mqttLabel, noCallback);
  ESPUI.setElementStyle(mqtt_pass_text, espuiTextSetupStyle);
  ESPUI.setInputType(mqtt_pass_text, "password");
  auto mqttenableLabel = ESPUI.addControl(Label, "", "Enable: ", None, mqttLabel, noCallback);
  ESPUI.setElementStyle(mqttenableLabel, espuiSwtLabelStyle);  
  mqtt_enabled_switch = ESPUI.addControl(Switcher, "Enable MQTT", String(mqtt_enabled), Dark, mqttLabel, switchCallback);
  ESPUI.setElementStyle(mqtt_enabled_switch, getEspuiSwitchStyle(mqtt_enabled)); 

  auto mqttTopicInLabel = ESPUI.addControl(Label, "MQTT Topics", "IN: ", None, wifitab, noCallback);
  ESPUI.setElementStyle(mqttTopicInLabel, espuiTextLabelStyle);  
  mqtt_topic_in_text = ESPUI.addControl(Text, "MQTT topic IN", stored_mqtt_topic_in, Dark, mqttTopicInLabel, noCallback);
  ESPUI.setElementStyle(mqtt_topic_in_text, espuiTextSetupStyle);
  auto mqttTopicOutLabel = ESPUI.addControl(Label, "", "OUT: ", None, mqttTopicInLabel, noCallback);
  ESPUI.setElementStyle(mqttTopicOutLabel, espuiTextLabelStyle);  
  mqtt_topic_out_text = ESPUI.addControl(Text, "MQTT topic OUT", stored_mqtt_topic_out, Dark, mqttTopicInLabel, noCallback);
  ESPUI.setElementStyle(mqtt_topic_out_text, espuiTextSetupStyle);

  auto wifisave = ESPUI.addControl(Button, "Save", "Save", Peterriver, wifitab, saveWifiDetailsCallback);
  auto espreset = ESPUI.addControl(Button, "", "Reboot turntable", None, wifisave, ESPReset);

  ESPUI.setEnabled(wifi_ssid_text, true);
  ESPUI.setEnabled(wifi_pass_text, true);
  ESPUI.setEnabled(mqtt_server_text, true);
  ESPUI.setEnabled(mqtt_user_text, true);
  ESPUI.setEnabled(mqtt_pass_text, true);
  ESPUI.setEnabled(mqtt_topic_in_text, true);
  ESPUI.setEnabled(mqtt_topic_out_text, true);
  ESPUI.setEnabled(mqtt_enabled_switch, true);
  ESPUI.setEnabled(wifisave, true);
  ESPUI.setEnabled(espreset, true);
  //WiFi-------------------------------------------------------------------------------------------------------------------

  ESPUI.begin(stored_devicename.c_str());
}
//ESPUI=====================================================================================================================
