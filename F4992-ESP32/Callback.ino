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
      uiToPosition = computePositionStepFromPercent(payload.toFloat());
      uiAskMoveTo = true;
    }
  }

  else if (topic.indexOf("tt_ffwd") > -1) {
    if (!isHome()) {
      uiToPosition = computePositionStepFromSeconds(constrain(currentPositionInSeconds() + ((float)getFfwdRewLenght() / 1000.0f), 0, approximateRecordLenght()));
      uiAskMoveTo = true;
    }
  }

  else if (topic.indexOf("tt_rew") > -1) {
    if (!isHome()) {
      uiToPosition = computePositionStepFromSeconds(constrain(currentPositionInSeconds() - ((float)getFfwdRewLenght() / 1000.0f), 0, approximateRecordLenght()));
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
        uiToPosition = computePositionStepFromSeconds(constrain(currentPositionInSeconds() + ((float)getFfwdRewLenght() / 1000.0f), 0, approximateRecordLenght()));
        uiAskMoveTo = true;
      }
    }
    else if (payload.indexOf("previous") > -1) {
      if (!isHome()) {
        uiToPosition = computePositionStepFromSeconds(constrain(currentPositionInSeconds() - ((float)getFfwdRewLenght() / 1000.0f), 0, approximateRecordLenght()));
        uiAskMoveTo = true;
      }
    }
    else {
      webSerialPrintln("media player unknown command");
      webSerialPrintln(topic);
      webSerialPrintln(payload);
    }
  }

  else {
    webSerialPrintln(topic);
    webSerialPrintln(payload);
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

void switchAdvancedSettingsCallback(Control *sender, int type) {
  writeAdvancedSettingsVisibilityy(type == S_ACTIVE);
  switchCallback(sender, type);
}


void switchWifiSettingsCallback(Control *sender, int type) {
  writeWifiSettingsVisibility(type == S_ACTIVE);
  switchCallback(sender, type);
}

void switchHardwareTuningCallback(Control *sender, int type) {
  writeHardwareTuningVisibility(type == S_ACTIVE);
  switchCallback(sender, type);
}


void switchDebugCallback(Control *sender, int type) {
  writeDebugVisibility(type == S_ACTIVE);
  switchCallback(sender, type);
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
void volMaxPwmCallback(Control *sender, int type) {
  if (type == T_VALUE) {
    storeMaxpwmvalue(sender->value.toInt());
  }
}

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
  if (!debugTabVisible()) return;
  ESPUI.updateControlValue(highVerbosity_switch, highVerbosity ? "1" : "0");
  ESPUI.setElementStyle(highVerbosity_switch, getEspuiSwitchStyle(highVerbosity));
}

//OTA link ===================================
void setEspuiFirmwareUpdateText() {
  if (uiAskfwupdate && isNetworkActive()) ESPUI.updateControlValue(firmwareUpdate, linkToOTA() );
  else ESPUI.updateControlValue(firmwareUpdate, "");
}
//OTA link ===================================

//Serial setup===============================================================
void SerialCommand(String input) {
  if (debugTabVisible()) ESPUI.print(serialLabelId, input);

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

  else if (input.indexOf("debugoff") > -1) {
    writeDebugVisibility(false);
  }

  else if (input.indexOf("debugon") > -1) {
    writeDebugVisibility(true);
  }

  else if (input.indexOf("cpu") > -1) {
    webSerialPrintln("--- CPU SYSTEM REPORT ---");
    webSerialPrintln("Model: " + getChipModel() + " (" + String(getCpuCores()) + " cores) @ " + String(ESP.getCpuFreqMHz()) + "MHz");
    webSerialPrintln("Temperature: " + String(getCpuTemperature(), 0) + "\xC2\xB0" + "C");
    webSerialPrintln("Last Boot: " + getReadableResetReason());
    webSerialPrintln("Last Crash: " + getReadableLastCrashReason());
    webSerialPrintln("RAM: " + String(getFreeHeap()) + " / " + String(getTotalHeap()) + " bytes free");
    webSerialPrintln("Flash: " + String(getSketchSize()) + " / " + String(getFlashSize()) + " bytes used");
    webSerialPrintln("Free Sketch: " + String(getFreeSketchSpace()) + " bytes");
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

  else if (input.indexOf("volume") > -1) {
    if (volumeChangerActivated()) {
      String volumeLevel = splitString(input, ' ', 1);
      webSerialPrintln("Volume to " + volumeLevel);
      setDesiredVolumePercent(volumeLevel.toInt());
    }
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
