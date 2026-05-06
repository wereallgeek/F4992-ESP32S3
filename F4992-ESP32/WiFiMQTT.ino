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
    client.setCallback(mqtt_callback);
  }
  WiFi.setSleep(false);
  Serial.print("\nIP address : ");
  Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
}
//WiFi================================================================================

//MQTT LOOP===================================
void mqtt_loop() {
  if (wificonnected && mqtt_enabled) {
    if (!client.connected()) {
      reconnect();
      return;
    }
    client.loop();
    
    client.publish(stored_mqtt_topic_out.c_str(), "ON", true);
  }
}
//MQTT LOOP===================================

//Discovery===================================================================
void sendDiscovery() {
  String discoveryTopic = "homeassistant/switch/" + stored_devicename + "/config";

  String payload = "{";
  payload += "\"name\":\"" + stored_devicename + "\",";
  payload += "\"stat_t\":\"" + stored_mqtt_topic_out + "\",";
  payload += "\"cmd_t\":\"" + stored_mqtt_topic_in + "\",";
  payload += "\"uniq_id\":\"" + stored_devicename + "_sw\"";
  payload += "}";
  Serial.println("test send");
  Serial.println(discoveryTopic);
  Serial.println(payload);

  client.publish(discoveryTopic.c_str(), payload.c_str(), true);
}
//Discovery===================================================================

//MQTT RECONNECT==============================================================
void reconnect() {
  if (millis() - last_millis > mqtt_retry_delay) {
    Serial.println("MQTT connection to : " + stored_mqtt_server);
    if (client.connect(stored_devicename.c_str(), stored_mqtt_user.c_str(), stored_mqtt_pass.c_str())) {
      Serial.println("MQTT connected !");

      //Home Assistant Discovery---------------------
      sendDiscovery();
      //SUBSCRIBE to Topics--------------------------
      client.subscribe(stored_mqtt_topic_in.c_str());
      //---------------------------------------------

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
