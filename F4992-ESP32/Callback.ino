//Custom callback================================
void CustomCallback(Control *sender, int type) {
  //Your code HERE !
}
//Custom callback================================

//MQTT CALLBACK===================================================
void mqtt_callback(String topic, byte *message, unsigned int length) {
  String messageTemp;
  //Read the Payload
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.println(messageTemp);

  if (topic == "demo_topic") {
    client.publish("response_topic", "PONG");
    Serial.println("Hello World");
  }

  //Custom action................................................
  //Your code HERE !
  //Custom action................................................
}
//MQTT CALLBACK===================================================


//Default ESPUI callback======================
void textCallback(Control *sender, int type) {
}
//Default ESPUI callback======================

//Turntable ESPUI callback========================
void buttonInCallback(Control *sender, int type) {
  if (type == B_DOWN) {
    requestMove(+5);
  }
}
void buttonOutCallback(Control *sender, int type) {
  if (type == B_DOWN) {
    requestMove(-5);
  }
}
void buttonUpdownCallback(Control *sender, int type) {
  if (type == B_DOWN) {
    requestUpDown();
  }
}void buttonStartStopCallback(Control *sender, int type) {
  if (type == B_DOWN) {
    requestPlayStop();
  }
}void buttonRepeatCallback(Control *sender, int type) {
  //nothing yet
}
//Turntable ESPUI callback========================



//config settings callback
void SaveTurntableDetailsCallback(Control *sender, int type) {
  if (type == B_UP) {
    stored_devicename = String(ESPUI.getControl(device_name_text)->value);
    highVerbosity = ESPUI.getControl(highVerbosity_switch)->value.toInt() ? true : false;
    preferences.putString("devicename", stored_devicename);
    preferences.putBool("highVerbosity", highVerbosity);
    Serial.println(stored_devicename);
    Serial.print("Verbosity ");  Serial.println(highVerbosity ? "HI" : "LO");
    Serial.println("Saving configuration");
  }
}

//WiFi settings callback=====================================================
void SaveWifiDetailsCallback(Control *sender, int type) {
  if (type == B_UP) {
    stored_ssid = ESPUI.getControl(wifi_ssid_text)->value;
    stored_pass = ESPUI.getControl(wifi_pass_text)->value;
    stored_mqtt_topic_in = ESPUI.getControl(mqtt_topic_in_text)->value;
    stored_mqtt_topic_out = ESPUI.getControl(mqtt_topic_out_text)->value;
    stored_mqtt_server = String(ESPUI.getControl(mqtt_server_text)->value);
    stored_mqtt_user = String(ESPUI.getControl(mqtt_user_text)->value);
    stored_mqtt_pass = String(ESPUI.getControl(mqtt_pass_text)->value);
    mqtt_enabled = ESPUI.getControl(mqtt_enabled_switch)->value.toInt() ? true : false;

    preferences.putString("ssid", stored_ssid);
    preferences.putString("pass", stored_pass);
    preferences.putString("mqtt_server", stored_mqtt_server);
    preferences.putString("mqtt_user", stored_mqtt_user);
    preferences.putString("mqtt_pass", stored_mqtt_pass);
    preferences.putString("mqtt_topic_in", stored_mqtt_topic_in);
    preferences.putString("mqtt_topic_out", stored_mqtt_topic_out);
    preferences.putBool("mqtt_enabled", mqtt_enabled);

    Serial.println(stored_ssid);
    Serial.println(stored_pass);
    Serial.println(stored_mqtt_server);
    Serial.println(stored_mqtt_user);
    Serial.println(stored_mqtt_pass);
    Serial.println(stored_mqtt_topic_in);
    Serial.println(stored_mqtt_topic_out);
    Serial.println(mqtt_enabled);

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

//Serial setup===============================================================
void SerialSetup(String input) {
  if (input.indexOf("ssid") > -1) {
    stored_ssid = splitString(input, ' ', 1);
    preferences.putString("ssid", stored_ssid);
    Serial.println("New SSID : " + stored_ssid);
  }

  else if (input.indexOf("password") > -1) {
    stored_pass = splitString(input, ' ', 1);
    preferences.putString("pass", stored_pass);
    Serial.println("New password : " + stored_pass);
  }

  else if (input.indexOf("mqtten") > -1) {
    mqtt_enabled = splitString(input, ' ', 1).toInt() ? true : false;
    preferences.putBool("mqtt_enabled", mqtt_enabled);
    Serial.println("MQTT enabled : " + String(mqtt_enabled));
  }

  else if (input.indexOf("mqttserver") > -1) {
    stored_mqtt_server = splitString(input, ' ', 1);
    preferences.putString("mqtt_server", stored_mqtt_server);
    Serial.println("New MQTT server : " + stored_mqtt_server);
  }

  else if (input.indexOf("mqttuser") > -1) {
    stored_mqtt_user = splitString(input, ' ', 1);
    preferences.putString("mqtt_user", stored_mqtt_user);
    Serial.println("New MQTT user : " + stored_mqtt_user);
  }

  else if (input.indexOf("mqttpass") > -1) {
    stored_mqtt_pass = splitString(input, ' ', 1);
    preferences.putString("mqtt_pass", stored_mqtt_pass);
    Serial.println("New MQTT pass : " + stored_mqtt_pass);
  }

  else if (input.indexOf("topicin") > -1) {
    stored_mqtt_topic_in = splitString(input, ' ', 1);
    preferences.putString("mqtt_topic_in", stored_mqtt_topic_in);
    Serial.println("New Topic IN : " + stored_mqtt_topic_in);
  }

  else if (input.indexOf("topicout") > -1) {
    stored_mqtt_topic_out = splitString(input, ' ', 1);
    preferences.putString("mqtt_topic_out", stored_mqtt_topic_out);
    Serial.println("New Topic OUT : " + stored_mqtt_topic_out);
  }

  else if (input.indexOf("restart") > -1) {
    ESP.restart();
  }

  else if (input.indexOf("report") > -1) {
    turntableReport();
  }

  else if (input.indexOf("info") > -1) {
    Serial.println("Device name " + stored_devicename);
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

  // else if (input.indexOf("custom_cmd") > -1) {
  //   String demo_cmd = splitString(input, ' ', 1);
  //   Serial.println(demo_cmd);
  //   preferences.putString("demo", "demo_cmd");
  // }
}
//Serial setup===================================================================
