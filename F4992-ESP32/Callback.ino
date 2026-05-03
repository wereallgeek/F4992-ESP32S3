//kitchenware
String firmwareVersion() {
  static String version = ""; 
  if (version != "") return version; //compute once print many

  static const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  version = String(__DATE__ + 7); 
  int m = (String(months).indexOf(String(__DATE__).substring(0, 3)) / 3) + 1;
  if (m < 10) version += "0";
  version += m;
  version += (__DATE__[4] == ' ') ? "0" : String(__DATE__[4]);
  version += __DATE__[5];
  version += "-";
  version += String(__TIME__).substring(0, 2);
  version += String(__TIME__).substring(3, 5);

  return version;
}

//Internal temperature sensor============================================================================================
void cpuTempSensorSetup() {
  temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(-10, 80);
  temperature_sensor_install(&temp_sensor_config, &temp_sensor);
  temperature_sensor_enable(temp_sensor);
}

float getCpuTemperature() {
  float tsens_out;
  temperature_sensor_get_celsius(temp_sensor, &tsens_out);
  return tsens_out;
}
//Internal temperature sensor============================================================================================

//MQTT CALLBACK===================================================
void mqtt_callback(String topic, byte *message, unsigned int length) {
  String messageTemp;
  //Read the Payload
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  webSerialPrintln(messageTemp);

  if (topic == "demo_topic") {
    client.publish("response_topic", "PONG");
    webSerialPrintln("Hello World");
  }

}
//MQTT CALLBACK===================================================


//Default ESPUI callback======================
void textCallback(Control *sender, int type) {
}
//Default ESPUI callback======================

//Turntable ESPUI callback========================
void buttonInCallback(Control *sender, int type) {
  if (type == B_DOWN) {
    uiPressMoveIn = true;
  }
  else   if (type == B_UP) {
    uiPressMoveIn = false;
  }
}

void buttonOutCallback(Control *sender, int type) {
  if (type == B_DOWN) {
    uiPressMoveOut = true;
  }
  else   if (type == B_UP) {
    uiPressMoveOut = false;
  }
}

void buttonUpdownCallback(Control *sender, int type) {
  if (type == B_UP) {
    uiPressUpDown = true;
  }
}

void buttonStartStopCallback(Control *sender, int type) {
  if (type == B_UP) {
    uiPressStartStop = true;
    uiTypeStartStop = WEB;
  }
}

void buttonRepeatCallback(Control *sender, int type) {
  if (type == B_UP) {
    uiPressRepeat = true;
  }
}

void switchCallback(Control *sender, int type) {
  ESPUI.setElementStyle(sender->id, (type == S_ACTIVE) ? swStyleON : swStyleOFF);
}

void buttonInvertCallback(Control *sender, int type) {
  uiInvert = (type == S_ACTIVE);
  switchCallback(sender, type);
}

void verbosityCallback(Control *sender, int type) {
  highVerbosity = (type == S_ACTIVE);
  switchCallback(sender, type);
}
//Turntable ESPUI callback========================
//config settings callback
void saveTurntableDetailsCallback(Control *sender, int type) {
  if (type == B_UP) {    
    webSerialPrintln("Saving turntable configuration");
    applyTurntableDetailsToMemory();
    saveTurntableDetailsToConfig();
    readTurntablePresetValuesFromStorage();
    outputTurntableDetailsValues();
    resyncTurntableDetailsToScreen();
  }
}

void applyTurntableDetailsCallback(Control *sender, int type) {
  if (type == B_UP) {    
    webSerialPrintln("Applying turntable configuration");
    applyTurntableDetailsToMemory();
    outputTurntableDetailsValues();
    resyncTurntableDetailsToScreen();
  }
}

void resetTurntableDetailsCallback(Control *sender, int type) {
  if (type == B_UP) {
    webSerialPrintln("Reset turntable to default");
    ttConfigClear();
    readTurntablePresetValuesFromStorage();
    saveTurntableDetailsToConfig();
    outputTurntableDetailsValues();
    resyncTurntableDetailsToScreen();
  }
}

//WiFi settings callback=====================================================
void saveWifiDetailsCallback(Control *sender, int type) {
  if (type == B_UP) {
    stored_devicename = String(ESPUI.getControl(device_name_text)->value);
    stored_ssid = ESPUI.getControl(wifi_ssid_text)->value;
    stored_pass = ESPUI.getControl(wifi_pass_text)->value;
    stored_mqtt_topic_in = ESPUI.getControl(mqtt_topic_in_text)->value;
    stored_mqtt_topic_out = ESPUI.getControl(mqtt_topic_out_text)->value;
    stored_mqtt_server = String(ESPUI.getControl(mqtt_server_text)->value);
    stored_mqtt_user = String(ESPUI.getControl(mqtt_user_text)->value);
    stored_mqtt_pass = String(ESPUI.getControl(mqtt_pass_text)->value);
    mqtt_enabled = ESPUI.getControl(mqtt_enabled_switch)->value.toInt() ? true : false;

    settings.putString("devicename", String(stored_devicename));
    settings.putString("ssid", String(stored_ssid));
    settings.putString("pass", String(stored_pass));
    settings.putString("mqtt_server", String(stored_mqtt_server));
    settings.putString("mqtt_user", String(stored_mqtt_user));
    settings.putString("mqtt_pass", String(stored_mqtt_pass));
    settings.putString("mqtt_topic_in", String(stored_mqtt_topic_in));
    settings.putString("mqtt_topic_out", String(stored_mqtt_topic_out));
    settings.putBool("mqtt_enabled", mqtt_enabled);

    if(highVerbosity) {
      webSerialPrintln(stored_devicename);
      webSerialPrintln(stored_ssid);
      webSerialPrintln(stored_pass);
      webSerialPrintln(stored_mqtt_server);
      webSerialPrintln(stored_mqtt_user);
      webSerialPrintln(stored_mqtt_pass);
      webSerialPrintln(stored_mqtt_topic_in);
      webSerialPrintln(stored_mqtt_topic_out);
      webSerialPrintln(mqtt_enabled);
    }

    webSerialPrintln("Saving settings");
  }
}
//WiFi settings callback=====================================================

//ESP Reset=================================
void ESPReset(Control *sender, int type) {
  if (type == B_UP) {
    ESP.restart();
  }
}
//ESP Reset=================================

//ESPUI command===============================
void commandCallback(Control* sender, int type) {
    if (type == T_VALUE) {
        SerialCommand(sender->value);
        ESPUI.updateControlValue(sender->id, ""); 
    }
}
//ESPUI command===============================

void refreshVerbosity() {
  ESPUI.updateControlValue(highVerbosity_switch, highVerbosity ? "1" : "0");
  ESPUI.setElementStyle(highVerbosity_switch, highVerbosity ? swStyleON : swStyleOFF);
}

//Serial setup===============================================================
void SerialCommand(String input) {
  ESPUI.print(serialLabelId, input);

  if (input.indexOf("ssid") > -1) {
    stored_ssid = splitString(input, ' ', 1);
    settings.putString("ssid", String(stored_ssid));
    webSerialPrintln("New SSID : " + stored_ssid);
  }

  else if (input.indexOf("password") > -1) {
    stored_pass = splitString(input, ' ', 1);
    settings.putString("pass", String(stored_pass));
    webSerialPrintln("New password : " + stored_pass);
  }

  else if (input.indexOf("mqtten") > -1) {
    mqtt_enabled = splitString(input, ' ', 1).toInt() ? true : false;
    settings.putBool("mqtt_enabled", mqtt_enabled);
    webSerialPrintln("MQTT enabled : " + String(mqtt_enabled));
  }

  else if (input.indexOf("mqttserver") > -1) {
    stored_mqtt_server = splitString(input, ' ', 1);
    settings.putString("mqtt_server", String(stored_mqtt_server));
    webSerialPrintln("New MQTT server : " + stored_mqtt_server);
  }

  else if (input.indexOf("mqttuser") > -1) {
    stored_mqtt_user = splitString(input, ' ', 1);
    settings.putString("mqtt_user", String(stored_mqtt_user));
    webSerialPrintln("New MQTT user : " + stored_mqtt_user);
  }

  else if (input.indexOf("mqttpass") > -1) {
    stored_mqtt_pass = splitString(input, ' ', 1);
    settings.putString("mqtt_pass", String(stored_mqtt_pass));
    webSerialPrintln("New MQTT pass : " + stored_mqtt_pass);
  }

  else if (input.indexOf("topicin") > -1) {
    stored_mqtt_topic_in = splitString(input, ' ', 1);
    settings.putString("mqtt_topic_in", String(stored_mqtt_topic_in));
    webSerialPrintln("New Topic IN : " + stored_mqtt_topic_in);
  }

  else if (input.indexOf("topicout") > -1) {
    stored_mqtt_topic_out = splitString(input, ' ', 1);
    settings.putString("mqtt_topic_out", String(stored_mqtt_topic_out));
    webSerialPrintln("New Topic OUT : " + stored_mqtt_topic_out);
  }

  input.toLowerCase();
  //handle lowercase entries to match with cellphone autocorrect
  if (input.indexOf("restart") > -1) {
    ESP.restart();
  }

  else if (input.indexOf("quiet") > -1) {
    highVerbosity = false;
    refreshVerbosity();
  }

  else if (input.indexOf("verbose") > -1) {
    highVerbosity = true;
    refreshVerbosity();
  }

  else if (input.indexOf("gohome") > -1) {
    uiAskMoveHome = true;
  }

  else if (input.indexOf("goend") > -1) {
    uiAskMoveEnd = true;
  }

  else if (input.indexOf("go30") > -1) {
    uiAskMove30 = true;
  }

  else if (input.indexOf("go17") > -1) {
    uiAskMove17 = true;
  }

  else if (input.indexOf("gostill") > -1) {
    uiAskMoveNot = true;
  }

  else if (input.indexOf("stop") > -1 || input.indexOf("start") > -1 || input.indexOf("play") > -1) {
    uiPressStartStop = true;
    uiTypeStartStop = SERIAL;
  }

  else if (input.indexOf("up") > -1 || input.indexOf("down") > -1 || input.indexOf("pause") > -1) {
    uiPressUpDown = true;
  }

  else if (input.indexOf("repeat") > -1) {
    uiPressRepeat = true;
  }

  else if (input.indexOf("bypass") > -1 || input.indexOf("overrite") > -1 || input.indexOf("init") > -1) {
    uiRequestInit = true;
  }

  else if (input.indexOf("temp") > -1) {
    float temperature = getCpuTemperature();
    webSerialPrintln("CPU Temperature: " + String(temperature, 0) + "\xC2\xB0" + "C");
  }

  else if (input.indexOf("report") > -1) {
    uiAskReport = true;
  }

  else if (input.indexOf("sensor") > -1) {
    uiAskInfra = true;
  }

  else if (input.indexOf("stats") > -1) {
    printStatsReport();
  }

  else if (input.indexOf("info") > -1) {
    webSerialPrintln("Device name " + stored_devicename);
    webSerialPrintln("Firmware " + firmwareVersion());
    webSerialPrintln("SSID " + stored_ssid);
    webSerialPrint("IP :");
    webSerialPrintln(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
    webSerialPrintln("MQTT server " + stored_mqtt_server);
    webSerialPrintln("MQTT user " + stored_mqtt_user);
    webSerialPrintln("MQTT enabled " + String(mqtt_enabled));
    webSerialPrintln("Topic IN " + stored_mqtt_topic_in);
    webSerialPrintln("Topic OUT " + stored_mqtt_topic_out);
    webSerialPrint("Verbosity ");  webSerialPrintln(highVerbosity ? "HI" : "LO");
  }

  else if (input.indexOf("help") > -1) {
    webSerialPrintln("-ssid, password + argument = WIFI");
    webSerialPrintln("-mqtten, mqttserver, mqttuser, mqttpass + argument = MQTT");
    webSerialPrintln("-topicin, topicout + argument = MQTT TOPIC");
    webSerialPrintln("-info, report, sensor, temp = reports");
    webSerialPrintln("-verbose, quiet = debug verbosity");
    webSerialPrintln("-go+[home/end/17/30/still] move arm");
    webSerialPrintln("-up/down/pause = up/down button");
    webSerialPrintln("-bypass/overrite/init = skip initialization and go IDLE");
    webSerialPrintln("-start/stop/play = start/stop button");
    webSerialPrintln("-repeat = repeat button  --  restart = reboot the device");
  }
}
//Serial setup===================================================================
