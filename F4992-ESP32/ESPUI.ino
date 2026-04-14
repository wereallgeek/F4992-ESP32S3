//ESPUI=====================================================================================================================
void espui_init() {
  ESPUI.setVerbosity(Verbosity::Quiet);//todo: link to settings
  
  //Tonearm Control - in header to be available from all tabs----------------------------------------------------------------------
  ledId = ESPUI.label("Controls", Dark, "");
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
  auto spd_switch = ESPUI.addControl(Switcher, "", String(softSpeedInverter), None, ledId, buttonInvertCallback);
  ESPUI.setElementStyle(spd_switch, swStyleOFF);

  //Turntable Controls-------------------------------------------------------------------------------------------------------------


  //Tonearm Status-----------------------------------------------------------------------------------------------------------------
  auto tonearmtab = ESPUI.addControl(Tab, "", "Status");
  armStatusLabelId = ESPUI.addControl(Label, "Status", "status", Carrot, tonearmtab, textCallback);
  armPositionLabelId = ESPUI.addControl(Text, "", "position", None, armStatusLabelId, textCallback);
  //Tonearm Status-----------------------------------------------------------------------------------------------------------------

  //Console debug tab--------------------------------------------------------------------------------------------------------------
  auto debugtab = ESPUI.addControl(Tab, "", "Debug");
  serialLabelId = ESPUI.addControl(Label, "Console", "last command", Dark, debugtab, textCallback);
  ESPUI.setElementStyle(serialLabelId, commandConsoleStyle);
  logLabelId = ESPUI.addControl(Label, "Console Log", "...", Dark, serialLabelId, textCallback);
  ESPUI.setElementStyle(logLabelId, commandConsoleStyle);
  auto cmd_input = ESPUI.addControl(Text, "Commande:", "", Dark, serialLabelId, commandCallback);
  ESPUI.setElementStyle(cmd_input, commandInputStyle);
  
  //Turntable configuration--------------------------------------------------------------------------------------------------------
  auto configtab = ESPUI.addControl(Tab, "", "Configuration");
  device_name_text = ESPUI.addControl(Text, "Device name", stored_devicename, Carrot, configtab, textCallback);
  highVerbosity_switch = ESPUI.addControl(Switcher, "High Verbosity", String(highVerbosity), Dark, configtab, verbosityCallback);
  auto configsave = ESPUI.addControl(Button, "Save", "Save", Peterriver, configtab, SaveTurntableDetailsCallback);
  auto espreboot = ESPUI.addControl(Button, "", "Reboot ESP", None, configsave, ESPReset);

  //WiFi---------------------------------------------------------------------------------------------------------------------------
  auto wifitab = ESPUI.addControl(Tab, "", "WiFi");
  wifi_ssid_text = ESPUI.addControl(Text, "SSID", stored_ssid, Carrot, wifitab, textCallback);
  wifi_pass_text = ESPUI.addControl(Text, "Password", stored_pass, Carrot, wifitab, textCallback);
  ESPUI.setInputType(wifi_pass_text, "password");
  ESPUI.addControl(Max, "", "32", None, wifi_ssid_text);
  ESPUI.addControl(Max, "", "64", None, wifi_pass_text);
  mqtt_server_text = ESPUI.addControl(Text, "MQTT server", stored_mqtt_server, Carrot, wifitab, textCallback);
  mqtt_user_text = ESPUI.addControl(Text, "MQTT user", stored_mqtt_user, Carrot, wifitab, textCallback);
  mqtt_pass_text = ESPUI.addControl(Text, "MQTT password", stored_mqtt_pass, Carrot, wifitab, textCallback);
  ESPUI.setInputType(mqtt_pass_text, "password");
  mqtt_enabled_switch = ESPUI.addControl(Switcher, "Enable MQTT", String(mqtt_enabled), Carrot, wifitab, textCallback);
  mqtt_topic_in_text = ESPUI.addControl(Text, "MQTT topic IN", stored_mqtt_topic_in, Carrot, wifitab, textCallback);
  mqtt_topic_out_text = ESPUI.addControl(Text, "MQTT topic OUT", stored_mqtt_topic_out, Carrot, wifitab, textCallback);

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
