//ESPUI=====================================================================================================================
void espui_init() {
  ESPUI.setVerbosity(Verbosity::Quiet);//todo: link to settings
  
  //Tonearm Control - in header to be available from all tabs----------------------------------------------------------------------
  ledId = ESPUI.label("<style>body { background-color: black; } .card { border-radius: 20px !important; }</style>Controls", Dark, "");
  ESPUI.setElementStyle(ledId, String("background-color: #2c3e50") + espuiIndicatorElementStyle);
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
  ESPUI.setElementStyle(repeatId, String("background-color: #2c3e50") + espuiIndicatorElementStyle);
  auto spd_switch = ESPUI.addControl(Switcher, "", String(softSpeedInverter), None, ledId, buttonInvertCallback);
  ESPUI.setElementStyle(spd_switch, swStyleOFF);
  //Turntable Controls-------------------------------------------------------------------------------------------------------------


  //Tonearm Status-----------------------------------------------------------------------------------------------------------------
  auto tonearmtab = ESPUI.addControl(Tab, "", "Status");
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
  auto debugtab = ESPUI.addControl(Tab, "", "Debug");
  serialLabelId = ESPUI.addControl(Label, "Console", "last command", Dark, debugtab, textCallback);
  ESPUI.setElementStyle(serialLabelId, commandConsoleStyle);
  logLabelId = ESPUI.addControl(Label, "Console Log", "...", Dark, serialLabelId, textCallback);
  ESPUI.setElementStyle(logLabelId, commandConsoleStyle);
  auto cmd_input = ESPUI.addControl(Text, "Commande:", "", Dark, serialLabelId, commandCallback);
  ESPUI.setElementStyle(cmd_input, commandInputStyle);
  auto highVerboLabel = ESPUI.addControl(Label, "", "High Verbosity", None, serialLabelId, textCallback);
  ESPUI.setElementStyle(highVerboLabel, espuiLongLabelStyle);  
  highVerbosity_switch = ESPUI.addControl(Switcher, "", String(highVerbosity), Dark, serialLabelId, verbosityCallback);
  ESPUI.setElementStyle(highVerbosity_switch, highVerbosity ? swStyleON : swStyleOFF);
  
  //Turntable configuration--------------------------------------------------------------------------------------------------------
  auto configtab = ESPUI.addControl(Tab, "", "Configuration");
  auto durationLabel = ESPUI.addControl(Label, "Duration", "detection: ", None, configtab, textCallback);
  ESPUI.setElementStyle(durationLabel, espuiLongLabelStyle);  
  detectionDurationLabelId = ESPUI.addControl(Number, "", String(getDetectionDuration()), Dark, durationLabel, textCallback);
  ESPUI.setElementStyle(detectionDurationLabelId, String("background-color: #2c3e50") + espuiNumberStyle);
  auto muteLabel = ESPUI.addControl(Label, "", "mute: ", None, durationLabel, textCallback);
  ESPUI.setElementStyle(muteLabel, espuiLongLabelStyle);  
  muteDurationLabelId = ESPUI.addControl(Number, "", String(getMuteDuration()), Dark, durationLabel, textCallback);
  ESPUI.setElementStyle(muteDurationLabelId, String("background-color: #2c3e50") + espuiNumberStyle);

  auto infraredLabel = ESPUI.addControl(Label, "Infrared", "duration: ", None, configtab, textCallback);
  ESPUI.setElementStyle(infraredLabel, espuiLongLabelStyle);  
  irCycleDurationLabelId = ESPUI.addControl(Number, "", String(getIrCycleDuration()), Dark, infraredLabel, textCallback);
  ESPUI.setElementStyle(irCycleDurationLabelId, String("background-color: #2c3e50") + espuiNumberStyle);
  auto tresholdLabel = ESPUI.addControl(Label, "", "treshold: ", None, infraredLabel, textCallback);
  ESPUI.setElementStyle(tresholdLabel, espuiLongLabelStyle);  
  irTresholdLabelId = ESPUI.addControl(Number, "", String(getIrTreshold()), Dark, infraredLabel, textCallback);
  ESPUI.setElementStyle(irTresholdLabelId, String("background-color: #2c3e50") + espuiNumberStyle);
  
  auto presetLabel = ESPUI.addControl(Label, "Arm Preset", "12\": ", None, configtab, textCallback);
  ESPUI.setElementStyle(presetLabel, espuiLabelStyle);  
  armPresetValue30LabelId = ESPUI.addControl(Number, "Arm Presets", String(getArmPresetValue(1)), Dark, presetLabel, textCallback);
  ESPUI.setElementStyle(armPresetValue30LabelId, String("background-color: #2c3e50") + espuiNumberStyle);
  auto preset17Label = ESPUI.addControl(Label, "", "6\": ", None, presetLabel, textCallback);
  ESPUI.setElementStyle(preset17Label, espuiLabelStyle);  
  armPresetValue17LabelId = ESPUI.addControl(Number, "", String(getArmPresetValue(2)), Dark, presetLabel, textCallback);
  ESPUI.setElementStyle(armPresetValue17LabelId, String("background-color: #2c3e50") + espuiNumberStyle);
  auto presetEndLabel = ESPUI.addControl(Label, "", "End: ", None, presetLabel, textCallback);
  ESPUI.setElementStyle(presetEndLabel, espuiLabelStyle);  
  armPresetValueEndLabelId = ESPUI.addControl(Number, "", String(getArmPresetValue(3)), Dark, presetLabel, textCallback);
  ESPUI.setElementStyle(armPresetValueEndLabelId, String("background-color: #2c3e50") + espuiNumberStyle);
  

  auto configsave = ESPUI.addControl(Button, "Save", "Save", Peterriver, configtab, SaveTurntableDetailsCallback);
  auto configApply = ESPUI.addControl(Button, "", "Apply", None, configsave, textCallback);
  
  //WiFi---------------------------------------------------------------------------------------------------------------------------
  auto wifitab = ESPUI.addControl(Tab, "", "WiFi");
  device_name_text = ESPUI.addControl(Text, "Device name", stored_devicename, Dark, wifitab, textCallback);
  wifi_ssid_text = ESPUI.addControl(Text, "SSID", stored_ssid, Dark, wifitab, textCallback);
  wifi_pass_text = ESPUI.addControl(Text, "Password", stored_pass, Dark, wifitab, textCallback);
  ESPUI.setInputType(wifi_pass_text, "password");
  ESPUI.addControl(Max, "", "32", None, wifi_ssid_text);
  ESPUI.addControl(Max, "", "64", None, wifi_pass_text);
  mqtt_server_text = ESPUI.addControl(Text, "MQTT server", stored_mqtt_server, Dark, wifitab, textCallback);
  mqtt_user_text = ESPUI.addControl(Text, "MQTT user", stored_mqtt_user, Dark, wifitab, textCallback);
  mqtt_pass_text = ESPUI.addControl(Text, "MQTT password", stored_mqtt_pass, Dark, wifitab, textCallback);
  ESPUI.setInputType(mqtt_pass_text, "password");
  mqtt_enabled_switch = ESPUI.addControl(Switcher, "Enable MQTT", String(mqtt_enabled), Dark, wifitab, textCallback);
  mqtt_topic_in_text = ESPUI.addControl(Text, "MQTT topic IN", stored_mqtt_topic_in, Dark, wifitab, textCallback);
  mqtt_topic_out_text = ESPUI.addControl(Text, "MQTT topic OUT", stored_mqtt_topic_out, Dark, wifitab, textCallback);

  auto wifisave = ESPUI.addControl(Button, "Save", "Save", Peterriver, wifitab, SaveWifiDetailsCallback);
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
