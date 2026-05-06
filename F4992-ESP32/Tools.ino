//Split String===========================================================
String splitString(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}
//Split String===========================================================

//Long Uptime============================================================
unsigned long getUptimeSeconds() {
  return (unsigned long)(esp_timer_get_time() / 1000000ULL);
}
//Long Uptime============================================================


//make an MQTT-safe name for topics======================================
String mqttId(String name) {
  String output = "";
  for (int i = 0; i < name.length(); i++) {
    char c = name[i];
    if (isAlphaNumeric(c)) {
      output += c;
    } else if (c == ' ' || c == '(' || c == ')' || c == '"' || c == ':' || c == '-') {
      if (output.length() > 0 && output[output.length() - 1] != '_') {
        output += '_';
      }
    }
  }
  if (output.endsWith("_")) {
    output.remove(output.length() - 1);
  }
  return output;
}
//make an MQTT-safe name for topics======================================