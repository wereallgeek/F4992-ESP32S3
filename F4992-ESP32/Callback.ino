//MQTT CALLBACK===================================================
void mqtt_callback(String topic, byte *message, unsigned int length) {
  String payload;
  //Read the Payload
  for (int i = 0; i < length; i++) {
    payload += (char)message[i];
  }

  topic.toLowerCase();

  if (topic.indexOf("restart_btn") > -1) {
    ESP.restart();
  }

  else if (topic.indexOf("gohome") > -1) {
    uiAskMoveHome = true;
  }

  else if (topic.indexOf("goend") > -1) {
    uiAskMoveEnd = true;
  }

  else if (topic.indexOf("go30") > -1) {
    uiAskMove30 = true;
  }

  else if (topic.indexOf("go17") > -1) {
    uiAskMove17 = true;
  }

  else if (topic.indexOf("gostill") > -1) {
    uiAskMoveNot = true;
  }

  else if (topic.indexOf("tt_startstop") > -1) {
    uiTypeStartStop = MQTT;
    uiPressStartStop = true;
  }

  else if (topic.indexOf("tt_updown") > -1) {
    uiPressUpDown = true;
  }

  else if (topic.indexOf("tt_mv_out") > -1) {
    if (payload.indexOf("ON") > -1) {
      uiPressMoveOut = true;
    }
    else if (payload.indexOf("OFF") > -1) {
      uiPressMoveOut = false;
    }
  }

  else if (topic.indexOf("tt_mv_in") > -1) {
    if (payload.indexOf("ON") > -1) {
      uiPressMoveIn = true;
    }
    else if (payload.indexOf("OFF") > -1) {
      uiPressMoveIn = false;
    }
  }

  else if (topic.indexOf("tt_rpt") > -1) {
    uiPressRepeat = true;
  }

  else if (topic.indexOf("tt_sft_inv") > -1) {
    if (payload.indexOf("ON") > -1) {
      uiInvert = true;
    }
    else if (payload.indexOf("OFF") > -1) {
      uiInvert = false;
    }
  }

  else if (topic.indexOf("zeroize") > -1) {
    statsReset();
  }

  else if (topic.indexOf("init") > -1) {
    uiRequestInit = true;
  }

  else if (topic.indexOf("tt_vol") > -1) {
    if (volumeChangerActivated()) {
      setDesiredVolumePercent(payload.toInt());
    }
  }

  else if (topic.indexOf("set_volume") > -1) {
    if (volumeChangerActivated()) {
      setDesiredVolumeZeroOne(payload.toFloat());
    }
  }

  else if (topic.indexOf("tt_arm_pct") > -1) {
    if (!isHome()) {
      uiToPosition = computePositionStepFromPercent(payload.toInt());
      uiAskMoveTo = true;
    }
  }

  else if (topic.indexOf("tt_ffwd") > -1) {
    if (!isHome()) {
      uiToPosition = computePositionStepFromSeconds(constrain(currentPositionInSeconds() + 3.0f, 0, approximateRecordLenght()));
      uiAskMoveTo = true;
    }
  }

  else if (topic.indexOf("tt_rew") > -1) {
    if (!isHome()) {
      uiToPosition = computePositionStepFromSeconds(constrain(currentPositionInSeconds() - 3.0f, 0, approximateRecordLenght()));
      uiAskMoveTo = true;
    }
  }

  else if (topic.indexOf("tt_play") > -1) {
    uiTypeStartStop = MQTT;
    uiPressJustStart = true;
  }

  else if (topic.indexOf("tt_stop") > -1) {
    uiTypeStartStop = MQTT;
    uiPressJustStop = true;
  }
  
  else if (topic.indexOf("media") > -1) {
    if (payload.indexOf("play") > -1) {
      uiTypeStartStop = MQTT;
      uiPressJustStart = true;
    }
    else if (payload.indexOf("pause") > -1) {
      uiPressUpDown = true;
    }
    else if (payload.indexOf("stop") > -1) {
      uiTypeStartStop = MQTT;
      uiPressJustStop = true;
    }
    else if (payload.indexOf("next") > -1) {
      if (!isHome()) {
        uiToPosition = computePositionStepFromSeconds(constrain(currentPositionInSeconds() + 3.0f, 0, approximateRecordLenght()));
        uiAskMoveTo = true;
      }
    }
    else if (payload.indexOf("previous") > -1) {
      if (!isHome()) {
        uiToPosition = computePositionStepFromSeconds(constrain(currentPositionInSeconds() - 3.0f, 0, approximateRecordLenght()));
        uiAskMoveTo = true;
      }
    }
    else {
      Serial.println("media player unknown command");
      Serial.println(topic);
      Serial.println(payload);
    }
  }

  else {
    Serial.println(topic);
    Serial.println(payload);
  }

}
//MQTT CALLBACK===================================================

//Default ESPUI callback======================
void noCallback(Control *sender, int type) {
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
    uiTypeStartStop = WEB;
    uiPressStartStop = true;
  }
}

void buttonRepeatCallback(Control *sender, int type) {
  if (type == B_UP) {
    uiPressRepeat = true;
  }
}

void buttonMoveHomeCallback(Control *sender, int type) {
  if (type == B_UP) {
    uiAskMoveHome = true;
  }
}

void buttonMoveEndCallback(Control *sender, int type) {
  if (type == B_UP) {
    uiAskMoveEnd = true;
  }
}

void buttonMove30Callback(Control *sender, int type) {
  if (type == B_UP) {
    uiAskMove30 = true;
  }
}

void buttonkMove17Callback(Control *sender, int type) {
  if (type == B_UP) {
    uiAskMove17 = true;
  }
}

void buttonkMoveNotCallback(Control *sender, int type) {
  if (type == B_UP) {
    uiAskMoveNot = true;
  }
}

void switchRepeatCallback(Control *sender, int type) {
  writeRepeatVisibility(type == S_ACTIVE);
  switchCallback(sender, type);
}

void switchInvertCallback(Control *sender, int type) {
  writeInvertVisibility(type == S_ACTIVE);
  switchCallback(sender, type);
}

void switchLedCallback(Control *sender, int type) {
  changeLedPixelEnable(type == S_ACTIVE);
  switchCallback(sender, type);
}

void switchCallback(Control *sender, int type) {
  reflectSwitchPosition(sender->id, (type == S_ACTIVE));
}

void numLedCallback(Control *sender, int type) {
  if (type == T_VALUE) {
    changeLedPixelNumber(sender->value.toInt());
  }
}

void ledBrightnessCallback(Control *sender, int type) {
  if (type == T_VALUE) {
    changeDesiredBrightness(sender->value.toInt());
  }
}

void ledchaserCallback(Control *sender, int type) {
  if (type == T_VALUE) {
    changeChaserSpeed(sender->value.toInt());
  }
}

void ledbreatherCallback(Control *sender, int type) {
  if (type == T_VALUE) {
    changeBreatherAdjustment(sender->value.toInt());
  }
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

//volume control setup callback===================
void volMinPwmCallback(Control *sender, int type) {
  if (type == T_VALUE) {
    storeMinpwmvalue(sender->value.toInt());
  }
}

void switchVolActCallback(Control *sender, int type) {
  storeVolumeControlEnable(type == S_ACTIVE);
  switchCallback(sender, type);
}
//volume control setup callback===================


//config settings callback
void saveTurntableDetailsCallback(Control *sender, int type) {
  if (type == B_UP) {    
    Serial.println("Saving turntable configuration");
    applyTurntableDetailsToMemory();
    saveTurntableDetailsToConfig();
    readTurntablePresetValuesFromStorage();
    outputTurntableDetailsValues();
    resyncTurntableDetailsToScreen();
  }
}

void applyTurntableDetailsCallback(Control *sender, int type) {
  if (type == B_UP) {    
    Serial.println("Applying turntable configuration");
    applyTurntableDetailsToMemory();
    outputTurntableDetailsValues();
    resyncTurntableDetailsToScreen();
  }
}

void resetTurntableDetailsCallback(Control *sender, int type) {
  if (type == B_UP) {
    Serial.println("Reset turntable to default");
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
      Serial.println(stored_devicename);
      Serial.println(stored_ssid);
      Serial.println(stored_pass);
      Serial.println(stored_mqtt_server);
      Serial.println(stored_mqtt_user);
      Serial.println(stored_mqtt_pass);
      Serial.println(stored_mqtt_topic_in);
      Serial.println(stored_mqtt_topic_out);
      Serial.println(mqtt_enabled);
    }

    Serial.println("Saving settings");
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

//OTA link ===================================
void setEspuiFirmwareUpdateText() {
  if (uiAskfwupdate && isNetworkActive()) ESPUI.updateControlValue(firmwareUpdate, linkToOTA() );
  else ESPUI.updateControlValue(firmwareUpdate, "");
}
//OTA link ===================================

//Serial setup===============================================================
void SerialCommand(String input) {
  if (input.indexOf("ssid") > -1) {
    stored_ssid = splitString(input, ' ', 1);
    settings.putString("ssid", String(stored_ssid));
    Serial.println("New SSID : " + stored_ssid);
  }

  else if (input.indexOf("password") > -1) {
    stored_pass = splitString(input, ' ', 1);
    settings.putString("pass", String(stored_pass));
    Serial.println("New password : " + stored_pass);
  }

  else if (input.indexOf("mqtten") > -1) {
    mqtt_enabled = splitString(input, ' ', 1).toInt() ? true : false;
    settings.putBool("mqtt_enabled", mqtt_enabled);
    Serial.println("MQTT enabled : " + String(mqtt_enabled));
  }

  else if (input.indexOf("mqttserver") > -1) {
    stored_mqtt_server = splitString(input, ' ', 1);
    settings.putString("mqtt_server", String(stored_mqtt_server));
    Serial.println("New MQTT server : " + stored_mqtt_server);
  }

  else if (input.indexOf("mqttuser") > -1) {
    stored_mqtt_user = splitString(input, ' ', 1);
    settings.putString("mqtt_user", String(stored_mqtt_user));
    Serial.println("New MQTT user : " + stored_mqtt_user);
  }

  else if (input.indexOf("mqttpass") > -1) {
    stored_mqtt_pass = splitString(input, ' ', 1);
    settings.putString("mqtt_pass", String(stored_mqtt_pass));
    Serial.println("New MQTT pass : " + stored_mqtt_pass);
  }

  else if (input.indexOf("topicin") > -1) {
    stored_mqtt_topic_in = splitString(input, ' ', 1);
    settings.putString("mqtt_topic_in", String(stored_mqtt_topic_in));
    Serial.println("New Topic IN : " + stored_mqtt_topic_in);
  }

  else if (input.indexOf("topicout") > -1) {
    stored_mqtt_topic_out = splitString(input, ' ', 1);
    settings.putString("mqtt_topic_out", String(stored_mqtt_topic_out));
    Serial.println("New Topic OUT : " + stored_mqtt_topic_out);
  }

  input.toLowerCase();
  //handle lowercase entries to match with cellphone autocorrect
  if (input.indexOf("restart") > -1) {
    ESP.restart();
  }

  else if (input.indexOf("quiet") > -1) {
    highVerbosity = false;
  }

  else if (input.indexOf("verbose") > -1) {
    highVerbosity = true;
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
    uiTypeStartStop = SERIAL;
    uiPressStartStop = true;
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

  else if (input.indexOf("cpu") > -1) {
    Serial.println("--- CPU SYSTEM REPORT ---");
    Serial.println("Model: " + getChipModel() + " (" + String(getCpuCores()) + " cores) @ " + String(ESP.getCpuFreqMHz()) + "MHz");
    Serial.println("Temperature: " + String(getCpuTemperature(), 0) + "\xC2\xB0" + "C");
    Serial.println("Last Boot: " + getReadableResetReason());
    Serial.println("Last Crash: " + getReadableLastCrashReason());
    Serial.println("RAM: " + String(getFreeHeap()) + " / " + String(getTotalHeap()) + " bytes free");
    Serial.println("Flash: " + String(getSketchSize()) + " / " + String(getFlashSize()) + " bytes used");
    Serial.println("Free Sketch: " + String(getFreeSketchSpace()) + " bytes");
  }

  else if (input.indexOf("temp") > -1) {
    float temperature = getCpuTemperature();
    Serial.println("CPU Temperature: " + String(temperature, 0) + "\xC2\xB0" + "C");
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

  else if (input.indexOf("volume") > -1) {
    if (volumeChangerActivated()) {
      String volumeLevel = splitString(input, ' ', 1);
      Serial.println("Volume to " + volumeLevel);
      setDesiredVolumePercent(volumeLevel.toInt());
    }
  }

  else if (input.indexOf("info") > -1) {
    Serial.println("Device name " + stored_devicename);
    Serial.println("Firmware " + firmwareVersion());
    Serial.println("SSID " + stored_ssid);
    Serial.print("IP :");
    Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
    Serial.println("MQTT server " + stored_mqtt_server);
    Serial.println("MQTT user " + stored_mqtt_user);
    Serial.println("MQTT enabled " + String(mqtt_enabled));
    Serial.println("Topic IN " + stored_mqtt_topic_in);
    Serial.println("Topic OUT " + stored_mqtt_topic_out);
    Serial.print("Verbosity ");  Serial.println(highVerbosity ? "HI" : "LO");
  }

  else if (input.indexOf("help") > -1) {
    Serial.println("-ssid, password + argument = WIFI");
    Serial.println("-mqtten, mqttserver, mqttuser, mqttpass + argument = MQTT");
    Serial.println("-topicin, topicout + argument = MQTT TOPIC");
    Serial.println("-info, report, sensor, temp = reports");
    Serial.println("-verbose, quiet = debug verbosity");
    Serial.println("-go+[home/end/17/30/still] move arm");
    Serial.println("-up/down/pause = up/down button");
    Serial.println("-bypass/overrite/init = skip initialization and go IDLE");
    Serial.println("-start/stop/play = start/stop button");
    Serial.println("-repeat = repeat button  --  restart = reboot the device");
  }
}
//Serial setup===================================================================
