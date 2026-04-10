//================================================
// Turntable Controller
//
// Handles hardware automation 
//    for full tonearm and platter control
//
//================================================

uint16_t armPosition;

uint16_t getArmPosition() {
  return armPosition;
}

//temp debug method
void updateArmPosition(uint16_t newPosition) {
  armPosition = newPosition;
}

//Tonearm control=============================
void turntableSetup() {
  updateArmStatus("idle");
  armPosition = 0;
}

void turntableLoop() {

}
//Tonearm control=============================
