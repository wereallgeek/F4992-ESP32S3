#include <map>
std::map<String, String> lastPublishedValues;
std::map<String, unsigned long> lastPublishTimes;

//WiFi================================================================================
void wifi_init() {
  stored_ssid = settings.getString("ssid", "SSID");
  stored_pass = settings.getString("pass", "PASSWORD");
  stored_mqtt_server = settings.getString("mqtt_server", "192.168.0.10");
  stored_mqtt_user = settings.getString("mqtt_user", "");
  stored_mqtt_pass = settings.getString("mqtt_pass", "");
  stored_mqtt_topic_in = settings.getString("mqtt_topic_in", "f4992esp32/incoming");
  stored_mqtt_topic_out = settings.getString("mqtt_topic_out", "f4992esp32/outgoing");
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
    MDNS.begin(stored_devicename.c_str());
    MDNS.addService("http", "tcp", 80);
    client.setServer(stored_mqtt_server.c_str(), 1883);
    client.setBufferSize(512); 
    client.setCallback(mqtt_callback);
  }
  WiFi.setSleep(false);
  Serial.print("\nIP address : ");
  Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
}
//WiFi================================================================================

//Topic Out===================================================================
void publishData(String suffix, String value, unsigned long minInterval = 250) {
  unsigned long now = millis();
  if ((lastPublishedValues[suffix] != value) && (now - lastPublishTimes[suffix] >= minInterval)) {
    String topic = stored_mqtt_topic_out + "/" + suffix;
    if (client.publish(topic.c_str(), value.c_str(), true)) {
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
    client.loop();
    
    publishData("test_sw", "ON", 250);
    publishData("cpu_temp", String(getCpuTemperature()), 1000);
    publishAllStats();
  }
}
//MQTT LOOP===================================

//Discovery===================================================================
void addEntity(String type, String name, String suffix, String dev_cla = "", String unit = "", String stat_cla = "", String ent_cat = "", String icon = "", bool enabled = true) {
  String entityId = mqttId(name); 

  String cleanName = name;
  cleanName.replace("\"", "\\\""); 
  
  String discoveryTopic = "homeassistant/" + type + "/" + stored_devicename + "_" + entityId + "/config";
  String stateTopic = stored_mqtt_topic_out + "/" + suffix;
  String cmdTopic   = stored_mqtt_topic_in  + "/" + suffix;

  String payload = "{";
  payload += "\"name\":\"" + cleanName + "\",";
  payload += "\"stat_t\":\"" + stateTopic + "\",";
  
  if (icon != "") {
    payload += "\"ic\":\"" + icon + "\",";
  }

  if (!enabled) {
    payload += "\"enabled_by_default\":false,";
  }

  if (type == "switch" || type == "light" || type == "number") {
    payload += "\"cmd_t\":\"" + cmdTopic + "\",";
  }
  
  if (dev_cla != "")  payload += "\"dev_cla\":\"" + dev_cla + "\",";
  if (unit != "")     payload += "\"unit_of_meas\":\"" + unit + "\",";
  if (stat_cla != "") payload += "\"stat_cla\":\"" + stat_cla + "\",";
  if (ent_cat != "")  payload += "\"ent_cat\":\"" + ent_cat + "\",";

  payload += "\"uniq_id\":\"" + stored_devicename + "_" + entityId + "\",";
  
  payload += "\"dev\":{";
  payload += "\"ids\":[\"" + stored_devicename + "\"],";
  payload += "\"name\":\"" + stored_devicename + "\",";
  payload += "\"mf\":\"We're all Geeks\",";
  payload += "\"mdl\":\"F4992-ESP32S3 Turntable controller\",";
  payload += "\"cu\":\"http://" + WiFi.localIP().toString() + "\",";
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
      Serial.println("MQTT connected !");

      //Home Assistant Discovery--------------------
      addEntity("switch", "Test Switch", "test_sw");
      addEntity("sensor", "CPU Temp", "cpu_temp", "temperature", "C", "measurement", "diagnostic", "");
      addAllStatEntities();
      //SUBSCRIBE to Topics----------------------------------------------------
      client.subscribe((stored_mqtt_topic_in + "/#").c_str());
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
//Specific ===================================================================