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

uint16_t lastKnownArmPosition;
String armStatus;
bool armStatusDirty;


// Turntable user interface ==============================================
String armPositionStatus(uint16_t position) {
  char buffer[25];
  snprintf(buffer, sizeof(buffer), "Arm position: %u", position);
  return String(buffer);
}

String updateArmStatus(String newStatus) {
  armStatus = newStatus;
  armStatusDirty = true;
  return armStatus;
}

void turntableUiSetup() {
  //force redraw after initialization
  lastKnownArmPosition = -1;
}

void turntableUiUpdate(uint16_t armPosition) {
  // conditionnal ui update
  if (armStatusDirty) 
    ESPUI.print(armStatusLabelId, armStatus);
  if (armPosition != lastKnownArmPosition) 
    ESPUI.print(armPositionLabelId, armPositionStatus(armPosition));

  resyncUiStatusVariables(armPosition);
}

void resyncUiStatusVariables(uint16_t armPosition) {
  //resync
  lastKnownArmPosition = armPosition;
  armStatusDirty = false;
}
// Turntable user interface ==============================================
