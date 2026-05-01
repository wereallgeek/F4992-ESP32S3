//ESPUI=====================================================================================================================
void espui_init() {
  ESPUI.setVerbosity(Verbosity::Quiet);//todo: link to settings

  //create tabs here to get urls with #1, #2, etc.  
  auto tonearmtab = ESPUI.addControl(Tab, "", "Status");
  auto debugtab = ESPUI.addControl(Tab, "", "Debug");
  auto configtab = ESPUI.addControl(Tab, "", "Config");
  auto wifitab = ESPUI.addControl(Tab, "", "WiFi");

  //Tonearm Control - in header to be available from all tabs----------------------------------------------------------------------
  ledId = ESPUI.label(espuiMainpageStyle, Dark, "");
  ESPUI.setElementStyle(ledId, String("background-color: #2c3e50") + espuiIndElemStyle);
  auto in_button = ESPUI.addControl(Button, "", "< Left ", None, ledId, buttonInCallback);
  ESPUI.setElementStyle(in_button, btnStyle);
  auto out_button = ESPUI.addControl(Button, "", "Right >", None, ledId, buttonOutCallback);
  ESPUI.setElementStyle(out_button, btnStyle);
  auto up_button = ESPUI.addControl(Button, "", "Up/Down", None, ledId, buttonUpdownCallback);
  ESPUI.setElementStyle(up_button, btnStyle);
  auto ss_button = ESPUI.addControl(Button, "", "Start/Stop", None, ledId, buttonStartStopCallback);
  ESPUI.setElementStyle(ss_button, btnStyle);
  auto rpt_button = ESPUI.addControl(Button, "", "Repeat", None, ledId, buttonRepeatCallback);
  ESPUI.setElementStyle(rpt_button, btnStyle);
  repeatId = ESPUI.addControl(Label, "", "", Dark, ledId, textCallback);
  ESPUI.setElementStyle(repeatId, String("background-color: #2c3e50") + espuiIndElemStyle);
  auto spd_switch = ESPUI.addControl(Switcher, "", String(softSpeedInverter), None, ledId, buttonInvertCallback);
  ESPUI.setElementStyle(spd_switch, swStyleOFF);
  //Turntable Controls-------------------------------------------------------------------------------------------------------------


  //Tonearm Status-----------------------------------------------------------------------------------------------------------------
  //status
  recordsizeLabelId = ESPUI.addControl(Label, "", "status", Dark, tonearmtab, textCallback);
  ESPUI.setElementStyle(recordsizeLabelId, record33style);
  lifterStatusId = ESPUI.addControl(Label, "", "arm", None, recordsizeLabelId, textCallback);
  ESPUI.setElementStyle(lifterStatusId, espuiIconStyle);
  armPositionLabelId = ESPUI.addControl(Label, "", "position", None, recordsizeLabelId, textCallback);
  ESPUI.setElementStyle(armPositionLabelId, String("background-color: #2c3e50") + espuiNumberStyle);
  dcmStatusId = ESPUI.addControl(Label, "", "dcm", None, recordsizeLabelId, textCallback);
  ESPUI.setElementStyle(dcmStatusId, espuiIconStyle);
  armStatusLabelId = ESPUI.addControl(Label, "", "status", Dark, recordsizeLabelId, textCallback);
  ESPUI.setElementStyle(armStatusLabelId, String("background-color: #2c3e50") + espuiStatusStyle);
  //Tonearm Status-----------------------------------------------------------------------------------------------------------------

  //Console debug tab--------------------------------------------------------------------------------------------------------------
  serialLabelId = ESPUI.addControl(Label, "Console", "last command", Dark, debugtab, textCallback);
  ESPUI.setElementStyle(serialLabelId, commandConsoleStyle);
  logLabelId = ESPUI.addControl(Label, "Console Log", "...", Dark, serialLabelId, textCallback);
  ESPUI.setElementStyle(logLabelId, commandConsoleStyle);
  auto cmd_input = ESPUI.addControl(Text, "Command:", "", Dark, serialLabelId, commandCallback);
  ESPUI.setElementStyle(cmd_input, commandInputStyle);
  auto highVerboLabel = ESPUI.addControl(Label, "", "High Verbosity", None, serialLabelId, textCallback);
  ESPUI.setElementStyle(highVerboLabel, espuiSwtLabelStyle);  
  highVerbosity_switch = ESPUI.addControl(Switcher, "", String(highVerbosity), Dark, serialLabelId, verbosityCallback);
  ESPUI.setElementStyle(highVerbosity_switch, highVerbosity ? swStyleON : swStyleOFF);
  auto firmwareversionLabel = ESPUI.addControl(Label, "", String("       FW ver ") + firmwareVersion(), None, serialLabelId, textCallback);
  ESPUI.setElementStyle(firmwareversionLabel, espuiSwtLabelStyle);  

  //Turntable configuration--------------------------------------------------------------------------------------------------------
  auto durationLabel = ESPUI.addControl(Label, "Duration", "detection: ", None, configtab, textCallback);
  ESPUI.setElementStyle(durationLabel, espuiLongLabelStyle);  
  detectionDurationLabelId = ESPUI.addControl(Text, "", String(getDetectionDuration()), Dark, durationLabel, textCallback);
  ESPUI.setElementStyle(detectionDurationLabelId, espuiTelStyle);
  ESPUI.setInputType(detectionDurationLabelId, "tel");
  auto muteLabel = ESPUI.addControl(Label, "", "     mute: ", None, durationLabel, textCallback);
  ESPUI.setElementStyle(muteLabel, espuiLongLabelStyle);  
  muteDurationLabelId = ESPUI.addControl(Text, "", String(getMuteDuration()), Dark, durationLabel, textCallback);
  ESPUI.setElementStyle(muteDurationLabelId, espuiTelStyle);
  ESPUI.setInputType(muteDurationLabelId, "tel");

  auto infraredLabel = ESPUI.addControl(Label, "Infrared", "duration: ", None, configtab, textCallback);
  ESPUI.setElementStyle(infraredLabel, espuiLongLabelStyle);  
  irCycleDurationLabelId = ESPUI.addControl(Text, "", String(getIrCycleDuration()), Dark, infraredLabel, textCallback);
  ESPUI.setElementStyle(irCycleDurationLabelId, espuiTelStyle);
  ESPUI.setInputType(irCycleDurationLabelId, "tel");
  auto tresholdLabel = ESPUI.addControl(Label, "", "treshold: ", None, infraredLabel, textCallback);
  ESPUI.setElementStyle(tresholdLabel, espuiLongLabelStyle);  
  irTresholdLabelId = ESPUI.addControl(Text, "", String(getIrTreshold()), Dark, infraredLabel, textCallback);
  ESPUI.setElementStyle(irTresholdLabelId, espuiTelStyle);
  ESPUI.setInputType(irTresholdLabelId, "tel");

  auto presetLabel = ESPUI.addControl(Label, "Arm Presets", "12\": ", None, configtab, textCallback);
  ESPUI.setElementStyle(presetLabel, espuiLabelStyle);  
  armPresetValue30LabelId = ESPUI.addControl(Text, "Arm Presets", String(getArmPresetValue(1)), Dark, presetLabel, textCallback);
  ESPUI.setElementStyle(armPresetValue30LabelId, espuiTelStyle);
  ESPUI.setInputType(armPresetValue30LabelId, "tel");
  auto preset17Label = ESPUI.addControl(Label, "", "6\": ", None, presetLabel, textCallback);
  ESPUI.setElementStyle(preset17Label, espuiLabelStyle);  
  armPresetValue17LabelId = ESPUI.addControl(Text, "", String(getArmPresetValue(2)), Dark, presetLabel, textCallback);
  ESPUI.setElementStyle(armPresetValue17LabelId, espuiTelStyle);
  ESPUI.setInputType(armPresetValue17LabelId, "tel");
  auto presetEndLabel = ESPUI.addControl(Label, "", "End: ", None, presetLabel, textCallback);
  ESPUI.setElementStyle(presetEndLabel, espuiLabelStyle);  
  armPresetValueEndLabelId = ESPUI.addControl(Text, "", String(getArmPresetValue(3)), Dark, presetLabel, textCallback);
  ESPUI.setElementStyle(armPresetValueEndLabelId, espuiTelStyle);
  ESPUI.setInputType(armPresetValueEndLabelId, "tel");
  
  auto configsave = ESPUI.addControl(Button, "Save", "Save", Peterriver, configtab, saveTurntableDetailsCallback);
  auto configApply = ESPUI.addControl(Button, "", "Apply", None, configsave, applyTurntableDetailsCallback);
  auto coinfigReset = ESPUI.addControl(Button, "", "Reset to default", None, configsave, resetTurntableDetailsCallback);

  //WiFi---------------------------------------------------------------------------------------------------------------------------
  device_name_text = ESPUI.addControl(Text, "Device name", stored_devicename, Dark, wifitab, textCallback);
  ESPUI.setElementStyle(device_name_text, espuiTextSetupStyle);

  auto wifiLabel = ESPUI.addControl(Label, "Wi-Fi setup", "SSID: ", None, wifitab, textCallback);
  ESPUI.setElementStyle(wifiLabel, espuiTextLabelStyle);  
  wifi_ssid_text = ESPUI.addControl(Text, "SSID", stored_ssid, Dark, wifiLabel, textCallback);
  ESPUI.setElementStyle(wifi_ssid_text, espuiTextSetupStyle);
  auto passLabel = ESPUI.addControl(Label, "", "pass: ", None, wifiLabel, textCallback);
  ESPUI.setElementStyle(passLabel, espuiTextLabelStyle);  
  wifi_pass_text = ESPUI.addControl(Text, "Password", stored_pass, Dark, wifiLabel, textCallback);
  ESPUI.setElementStyle(wifi_pass_text, espuiTextSetupStyle);
  ESPUI.setInputType(wifi_pass_text, "password");
  ESPUI.addControl(Max, "", "32", None, wifi_ssid_text);
  ESPUI.addControl(Max, "", "64", None, wifi_pass_text);

  auto mqttLabel = ESPUI.addControl(Label, "MQTT setup", "IP: ", None, wifitab, textCallback);
  ESPUI.setElementStyle(mqttLabel, espuiTextLabelStyle);  
  mqtt_server_text = ESPUI.addControl(Text, "MQTT server", stored_mqtt_server, Dark, mqttLabel, textCallback);
  ESPUI.setElementStyle(mqtt_server_text, espuiTextSetupStyle);
  auto mqttuserLabel = ESPUI.addControl(Label, "", "User: ", None, mqttLabel, textCallback);
  ESPUI.setElementStyle(mqttuserLabel, espuiTextLabelStyle);  
  mqtt_user_text = ESPUI.addControl(Text, "MQTT user", stored_mqtt_user, Dark, mqttLabel, textCallback);
  ESPUI.setElementStyle(mqtt_user_text, espuiTextSetupStyle);
  auto mqttpasswordLabel = ESPUI.addControl(Label, "", "Pass: ", None, mqttLabel, textCallback);
  ESPUI.setElementStyle(mqttpasswordLabel, espuiTextLabelStyle);  
  mqtt_pass_text = ESPUI.addControl(Text, "MQTT password", stored_mqtt_pass, Dark, mqttLabel, textCallback);
  ESPUI.setElementStyle(mqtt_pass_text, espuiTextSetupStyle);
  ESPUI.setInputType(mqtt_pass_text, "password");
  auto mqttenableLabel = ESPUI.addControl(Label, "", "Enable: ", None, mqttLabel, textCallback);
  ESPUI.setElementStyle(mqttenableLabel, espuiSwtLabelStyle);  
  mqtt_enabled_switch = ESPUI.addControl(Switcher, "Enable MQTT", String(mqtt_enabled), Dark, mqttLabel, switchCallback);
  ESPUI.setElementStyle(mqtt_enabled_switch, mqtt_enabled ? swStyleON : swStyleOFF);

  auto mqttTopicInLabel = ESPUI.addControl(Label, "MQTT Topics", "IN: ", None, wifitab, textCallback);
  ESPUI.setElementStyle(mqttTopicInLabel, espuiTextLabelStyle);  
  mqtt_topic_in_text = ESPUI.addControl(Text, "MQTT topic IN", stored_mqtt_topic_in, Dark, mqttTopicInLabel, textCallback);
  ESPUI.setElementStyle(mqtt_topic_in_text, espuiTextSetupStyle);
  auto mqttTopicOutLabel = ESPUI.addControl(Label, "", "OUT: ", None, mqttTopicInLabel, textCallback);
  ESPUI.setElementStyle(mqttTopicOutLabel, espuiTextLabelStyle);  
  mqtt_topic_out_text = ESPUI.addControl(Text, "MQTT topic OUT", stored_mqtt_topic_out, Dark, mqttTopicInLabel, textCallback);
  ESPUI.setElementStyle(mqtt_topic_out_text, espuiTextSetupStyle);

  auto wifisave = ESPUI.addControl(Button, "Save", "Save", Peterriver, wifitab, saveWifiDetailsCallback);
  auto espreset = ESPUI.addControl(Button, "", "Reboot ESP", None, wifisave, ESPReset);

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
