#include <map>
std::map<String, String> lastPublishedValues;
std::map<String, unsigned long> lastPublishTimes;

bool firsttime = true;

time_t bootEpoch = 0; 
bool timeSynced = false;

bool sendOnReconnect = false;

//stats
enum wirelessStatsType {
  NBWIFICONNECT, NBMQTTCONNECT, 
  MAXWIRELESS
};

const char* wirelessStatsKeys[] = {
  "nbcon_wifi", "nbcon_mqtt"
};

const char* wirelessStatsLabels[] = {
  "WiFi Connect Count", "MQTT Connect Count"
};

volatile uint32_t wirelessNumberStats[MAXWIRELESS];

String computeUTCTime() {
  if (!timeSynced)return "";

  char isoTime[25];
  struct tm *dt;
  dt = gmtime(&bootEpoch);
  strftime(isoTime, sizeof(isoTime), "%Y-%m-%dT%H:%M:%SZ", dt);
  return String(isoTime);
}

void incrementWirelessStat(int type) {
  wirelessNumberStats[type]++; 
}
//WiFi================================================================================
void setWifiSleep(bool sleepEnable) {
  WiFi.setSleep(sleepEnable);
}

bool isNetworkActive() {
  return (isWifiConnected()|| isAnAccessPoint());
}

bool isWifiConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

bool isAnAccessPoint() {
  return (WiFi.getMode() & WIFI_AP);
}

String getNetworkIpAddress() {
  if (isAnAccessPoint()) return WiFi.softAPIP().toString();
  else if (isWifiConnected()) return WiFi.localIP().toString();
  else return "";
}


void wifi_init() {
  for (int i = 0; i < MAXWIRELESS; i++) wirelessNumberStats[i] = 0;
  stored_ssid = settings.getString("ssid", "SSID");
  stored_pass = settings.getString("pass", "PASSWORD");
  stored_mqtt_server = settings.getString("mqtt_server", "192.168.0.10");
  stored_mqtt_user = settings.getString("mqtt_user", "");
  stored_mqtt_pass = settings.getString("mqtt_pass", "");
  stored_mqtt_topic_in = settings.getString("mqtt_topic_in", "f4992esp32-listen");
  stored_mqtt_topic_out = settings.getString("mqtt_topic_out", "f4992esp32-talk");
  mqtt_enabled = settings.getBool("mqtt_enabled", false);

  //devicename  
  stored_devicename = settings.getString("devicename", hostname); 
  
  Serial.println("Connecting to : " + stored_ssid);
  WiFi.begin(stored_ssid.c_str(), stored_pass.c_str());
  uint8_t timeout = 30;
  while (timeout && WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    timeout--;
  }
  if (WiFi.status() != WL_CONNECTED) {
    wificonnected = false;
    Serial.print("\n\nCreating Hotspot");
    WiFi.mode(WIFI_AP);
    delay(100);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(stored_devicename.c_str());
    dnsServer.start(DNS_PORT, "*", apIP);
  } else {
    wificonnected = true;
    configTime(0, 0, "pool.ntp.org", "://google.com");
    //world time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      bootEpoch = mktime(&timeinfo);
      timeSynced = true;
    }
    //
    MDNS.begin(stored_devicename.c_str());
    MDNS.addService("http", "tcp", 80);
    client.setServer(stored_mqtt_server.c_str(), 1883);
    client.setBufferSize(1024); 
    client.setCallback(mqtt_callback);
    incrementWirelessStat(NBWIFICONNECT);
    uiAskfwupdate = true;
  }
  setWifiSleep(false);
  Serial.print("\nIP address : ");
  Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
}
//WiFi================================================================================

//Topic Out===================================================================
void publishData(String suffix, String value, unsigned long minInterval = 250, bool retain = true) {
  unsigned long now = millis();
  if (sendOnReconnect || ((lastPublishedValues[suffix] != value) && (now - lastPublishTimes[suffix] >= minInterval))) {
    String topic = stored_mqtt_topic_out + "/" + suffix;
    if (client.publish(topic.c_str(), value.c_str(), retain)) {
      lastPublishedValues[suffix] = value;
      lastPublishTimes[suffix] = now;
    }
  }
}
//Topic Out===================================================================

//MQTT LOOP===================================
void mqtt_loop() {
  if (wificonnected && mqtt_enabled) {
    if (!client.connected()) {
      reconnect();
      return;
    }
    else
    {
      publishTurntableData();
      publishVolumeData();
      publishAllStats();
      publishAllhWirelessStats();
      publisAllCpuInfo();
      sendOnReconnect = false;
    }
    client.loop();
  }
}
//MQTT LOOP===================================

//Discovery===================================================================
void addEntity(String type, String name, String suffix, String deviceClass = "", String unit = "", String stateClass = "", String entityClass = "", String icon = "", bool enabled = true) {
  String entityId = mqttId(name); 

  String cleanName = name;
  cleanName.replace("\"", "\\\""); 
  
  String discoveryTopic = "homeassistant/" + type + "/" + stored_devicename + "_" + entityId + "/config";
  String stateTopic = stored_mqtt_topic_out + "/" + suffix;
  String cmdTopic   = stored_mqtt_topic_in  + "/" + suffix;

  String payload = "{";
  payload += "\"name\":\"" + cleanName + "\",";
  if (type == "media_player") {
    payload += "\"availability\":{";
    payload += "\"topic\":\"" + stored_mqtt_topic_out + "/available\",";
    payload += "\"payload_available\":\"Online\",";
    payload += "\"payload_not_available\":\"Offline\"";
    payload += "},";

    payload += "\"state_state_topic\":\"" + stateTopic + "\",";
    payload += "\"state_title_topic\":\"" + stored_mqtt_topic_out + "/tt_status\",";
    payload += "\"state_duration_topic\":\"" + stored_mqtt_topic_out + "/media_duration\",";
    payload += "\"state_position_topic\":\"" + stored_mqtt_topic_out + "/media_position\",";
    if (volumeChangerActivated()) {
      payload += "\"state_volume_topic\":\"" + stored_mqtt_topic_out + "/get_volume\",";
      payload += "\"command_volume_topic\":\"" + stored_mqtt_topic_in + "/set_volume\",";
    }
    payload += "\"command_play_topic\":\"" + cmdTopic + "\",";
    payload += "\"command_play_payload\":\"play\",";
    payload += "\"command_pause_topic\":\"" + stored_mqtt_topic_in + "/tt_updown\",";
    payload += "\"command_pause_payload\":\"\",";
    payload += "\"command_stop_topic\":\"" + cmdTopic + "\",";
    payload += "\"command_stop_payload\":\"stop\",";
  }
  else {
    if (type != "button") {
      payload += "\"stat_t\":\"" + stateTopic + "\",";
    }
    
    if (type == "binary_sensor" || type == "switch") {
      payload += "\"pl_on\":\"ON\",";
      payload += "\"pl_off\":\"OFF\",";
    }

    if (type == "switch" || type == "light" || type == "number" || type == "button") {
      payload += "\"cmd_t\":\"" + cmdTopic + "\",";
    }
  }

  if (icon != "") {
    payload += "\"ic\":\"" + icon + "\",";
  }

  if (!enabled) {
    payload += "\"enabled_by_default\":false,";
  }
  
  if (deviceClass != "")   payload += "\"dev_cla\":\"" + deviceClass + "\",";
  if (unit != "")          payload += "\"unit_of_meas\":\"" + unit   + "\",";
  if (stateClass != "")    payload += "\"stat_cla\":\"" + stateClass + "\",";
  if (entityClass != "")   payload += "\"ent_cat\":\"" + entityClass + "\",";
  

  payload += "\"uniq_id\":\"" + stored_devicename + "_" + entityId + "\",";
  
  payload += "\"dev\":{";
  payload += "\"ids\":[\"" + stored_devicename + "\"],";
  payload += "\"name\":\"" + stored_devicename + "\",";
  payload += "\"mf\":\"We're all Geeks\",";
  payload += "\"mdl\":\"Turntable controller\",";
  payload += "\"cu\":\"http://" + WiFi.localIP().toString() + "\",";
  payload += "\"hw\":\"F4992-ESP32-S3\",";
  payload += "\"sw\":\"" + firmwareVersion() + "\"";
  payload += "}";
  
  payload += "}";

  client.publish(discoveryTopic.c_str(), payload.c_str(), true);
}
//Discovery===================================================================

//MQTT RECONNECT==============================================================
void reconnect() {
  if (millis() - last_millis > mqtt_retry_delay) {
    Serial.println("MQTT connection to : " + stored_mqtt_server);
    if (client.connect(stored_devicename.c_str(), stored_mqtt_user.c_str(), stored_mqtt_pass.c_str())) {
      Serial.println("MQTT connected");
      incrementWirelessStat(NBMQTTCONNECT);

      lastPublishedValues.clear();
      lastPublishTimes.clear();
      sendOnReconnect = true;

      //SUBSCRIBE to Topics-----------------------------------
      client.subscribe((stored_mqtt_topic_in + "/#").c_str());
      //Home Assistant Discovery------------------------------
      addTurntableEntities();
      addVolumeEntities();
      addAllStatEntities();
      addAllWirelessStatEntities();
      addAllCpuInfoEntities();
      //------------------------------------------------------

    } else {
      Serial.print("MQTT connection failed : ");
      Serial.println(client.state());
      Serial.println("Retry in 10 sec");
      last_millis = millis();
      return;
    }
  }
}
//MQTT RECONNECT==============================================================

//Specific ===================================================================
void addTurntableEntities() {
  //addEntity( type,        name,         suffix,         deviceClass unit     stateClass     ent  icon                     enabled)
  addEntity("media_player", "Turntable",    "media",        "",         "",       "",            "", "mdi:record-player");
  addEntity("switch",       "Repeat",       "tt_rpt",       "",         "",       "",            "", "mdi:repeat");
  addEntity("switch",       "Move In",      "tt_mv_in",     "",         "",       "",            "", "mdi:arrow-expand-left");
  addEntity("switch",       "Move Out",     "tt_mv_out",    "",         "",       "",            "", "mdi:arrow-expand-right");
  addEntity("button",       "Up/Down",      "tt_updown",    "",         "",       "",            "", "mdi:arrow-expand-vertical");
  addEntity("button",       "Start/Stop",   "tt_startstop", "",         "",       "",            "", "mdi:record-player");
  addEntity("switch",       "Invert speed", "tt_sft_inv",   "",         "",       "",            "", "mdi:sync-circle");

  addEntity("sensor",       "Record Speed", "tt_spd",       "",         "rpm",    "measurement", "", "mdi:gauge");
  addEntity("sensor",       "Record Size",  "tt_size",      "",         "",       "",            "", "mdi:album");
  addEntity("sensor",       "Arm Lifter",   "tt_armlift",   "",         "",       "",            "", "mdi:arrow-expand-up", false);
  addEntity("sensor",       "Arm Position", "tt_armpos",    "distance", "steps",  "measurement", "", "mdi:pan-horizontal");
  addEntity("sensor",       "Status",       "tt_status",    "",         "",       "",            "", "mdi:information-slab-box-outline");
  addEntity("sensor",       "DCM status",   "tt_dcm",       "",         "",       "",            "", "mdi:cog-box",         false);
  
  addEntity("binary_sensor", "Arm Reset Switch", "tt_armlim",  "motion", "", "", "", "mdi:arrow-collapse-right");
}

void publishTurntableData() {
  publishData("tt_rpt",     getRepeatState()  ? "ON" : "OFF");
  publishData("tt_sft_inv", softSpeedInverter ? "ON" : "OFF");
  publishData("tt_mv_in",   uiPressMoveIn     ? "ON" : "OFF");
  publishData("tt_mv_out",  uiPressMoveOut    ? "ON" : "OFF");

  publishData("tt_spd",     getUiRecordSize());
  publishData("tt_size",    getUiSizeName());
  publishData("tt_armlift", (armLifter() == getArmUpLevel() ? "Raised" : "Lowered"));
  publishData("tt_armpos",  String(getUiArmPosition()));
  publishData("tt_status",  turntableCurrentStatus());
  publishData("tt_dcm",     String("DCM") + getUipreviousDcm());

  publishData("tt_armlim",  reachedArmReset() ? "ON" : "OFF");
  //mediaplayer
  publishData("available", "Online", 36000, true);
  publishData("media", turntableCurrentMediaplayerStatus(), 750); 
  publishData("media_duration", String(approximateRecordLenght(), 0), 10000);
  publishData("media_position", String(currentPositionInSeconds(), 1), 750);
}

void addVolumeEntities() {
  if (!volumeChangerActivated()) return;
  addEntity("number", "Volume", "tt_volume", "", "%", "", "", "mdi:volume-high");
  addEntity("sensor", "Computed Volume", "tt_c_vol", "", "%", "", "", "mdi:volume-source");
  addEntity("sensor", "Desired Volume", "tt_d_vol", "", "%", "measurement", "", "mdi:volume-equal", false);
  addEntity("sensor", "Raw Volume", "tt_raw_vol", "", "pwm", "measurement", "", "mdi:volume-vibrate", false);
  addEntity("sensor", "Volume Muting", "tt_vol_mute", "", "", "", "", "mdi:volume-mute", false);
}

void publishVolumeData() {
  if (!volumeChangerActivated()) return;
  publishData("tt_volume",   String(getDesiredVolumePercent()));
  publishData("get_volume",  String(getDesiredVolumeZeroOne(), 2));
  publishData("tt_c_vol",    String(getVolumePercent()));
  publishData("tt_d_vol",    String(getDesiredVolumePercent()));
  publishData("tt_raw_vol",  String(getVolumePWM()));
  publishData("tt_vol_mute", getVolumeMuteActivation() ? "ON" : "OFF");
}

void addAllStatEntities() {
  for (int statType = 0; statType < maxStatIndex(); statType++) {
    addEntity("sensor", getStatLabel(statType), getStatKey(statType), "", "", "total_increasing", (statType == maxStatIndex() - 1) ? "diagnostic" : "", getStatIcon(statType), (statType < 8) ? false : true);
  }
}

void publishAllStats() {
  for (int statType = 0; statType < maxStatIndex(); statType++) {
    publishData(getStatKey(statType), String(getStat(statType)));
  }
}

void addAllWirelessStatEntities() {
  for (int statType = 0; statType < MAXWIRELESS; statType++) {
    addEntity("sensor", wirelessStatsLabels[statType], wirelessStatsKeys[statType], "", "", "total_increasing", "diagnostic", "mdi:counter");
  }
  addEntity("sensor", "IP", "ip_addr", "", "", "", "diagnostic", "mdi:ip-network");
  addEntity("sensor", "SSID", "ssid", "", "", "", "diagnostic", "mdi:wifi");
  addEntity("sensor", "Last Restart Time", "restart_time", "timestamp", "", "", "diagnostic", "mdi:clock");
  addEntity("sensor", "WiFi Signal", "rssi", "signal_strength", "dBm", "measurement", "diagnostic", "mdi:wifi");
}

void publishAllhWirelessStats() {
  for (int statType = 0; statType < MAXWIRELESS; statType++) {
    publishData(wirelessStatsKeys[statType], String(wirelessNumberStats[statType]));
  }
  publishData("ip_addr", WiFi.localIP().toString());
  publishData("ssid", WiFi.SSID());
  publishData("restart_time", computeUTCTime(), 60000);
  publishData("rssi", String(WiFi.RSSI()), 30000);
}

void addAllCpuInfoEntities() {
  addEntity("sensor", "Restart Reason", "rst_reason", "", "", "", "diagnostic", "mdi:information-outline");
  addEntity("sensor", "Last Known Crash", "crash_reason", "", "", "", "diagnostic", "mdi:alert-octagon");
  addEntity("sensor", "CPU Temp", "cpu_temp", "temperature", "°C", "measurement", "diagnostic", "", false);
  addEntity("sensor", "CPU Model", "cpu_model", "", "", "", "diagnostic", "mdi:cpu-64-bit", false);
  addEntity("sensor", "CPU Cores", "cpu_cores", "", "", "", "diagnostic", "mdi:cpu-64-bit", false);
  addEntity("sensor", "CPU Frequency", "cpu_freq", "", "", "", "diagnostic", "mdi:cpu-64-bit", false);
  addEntity("sensor", "Free Heap", "free_heap", "data_size", "B", "measurement", "diagnostic", "mdi:memory", false);
  addEntity("sensor", "Total Heap", "total_heap", "data_size", "B", "", "diagnostic", "mdi:memory", false);
  addEntity("sensor", "Flash Size", "flash_size", "data_size", "B", "", "diagnostic", "mdi:harddisk", false);
  addEntity("sensor", "Sketch Size", "sketch_size", "data_size", "B", "", "diagnostic", "mdi:file-code-outline", false);
  addEntity("sensor", "Free Sketch Space", "free_sketch", "data_size", "B", "", "diagnostic", "mdi:file-code-outline", false);
  addEntity("button", "Restart", "restart_btn", "restart", "", "", "config", "mdi:restart");
}

void publisAllCpuInfo() {
  publishData("rst_reason", getReadableResetReason());
  publishData("crash_reason", getReadableLastCrashReason());
  publishData("cpu_temp", String(getCpuTemperature()), 60000);
  publishData("cpu_model", getChipModel());
  publishData("cpu_cores", String(getCpuCores()));
  publishData("cpu_freq", getCpuFreqMHz()); 
  publishData("free_heap", String(getFreeHeap()));
  publishData("total_heap", String(getTotalHeap()));
  publishData("flash_size", String(getFlashSize()));
  publishData("sketch_size", String(getSketchSize()));
  publishData("free_sketch", String(getFreeSketchSpace()));
}
//Specific ===================================================================