//========================================
// Turntable User Interface
//
// handles man-machine interfaces
//   - MQTT to home automation
//   - Web page interactions
//   - Physical buttons on the turntable
//
// Does not contain ESPUI specific data
//    such as field ID
//========================================
unsigned long lastUpdateMillis = 0; 


// Turntable user interface ==============================================
String armPositionStatus(uint16_t position) {
  char buffer[25];
  snprintf(buffer, sizeof(buffer), "Arm position: %u", position);
  return String(buffer);
}

void turntableUiSetup() {
}

void turntableUiUpdate(uint16_t armPosition) {
  // conditionnal ui update
  if (millis() - lastUpdateMillis >= 500) {
    lastUpdateMillis += 500;
    ESPUI.print(armStatusLabelId, turntableStatus());  
    ESPUI.print(armPositionLabelId, armPositionStatus(armPosition));
  }
}
// Turntable user interface ==============================================
