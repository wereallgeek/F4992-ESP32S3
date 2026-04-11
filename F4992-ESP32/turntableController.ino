//================================================
// Turntable Controller
//
// Handles hardware automation 
//    for full tonearm and platter control
//
//================================================

//todo: define appropriately for correct logic-level
const bool armUp = false;
const bool armDown = true;

const uint16_t  positionHome = 0;
const uint16_t  positionTo30 = 200; //todo: determine
const uint16_t  positionTo15 = 400; //todo: determine

enum TurntableState {IDLE, INITIAL, GOHOME, MOVE, DETECT, PLAY};
const char* TurntableStateDesc[] = {"Idle", "Initialization", "Going home", "Arm in motion", "Detection", "Playing"};
TurntableState currentState = IDLE;
TurntableState nextState = IDLE;
//prototype to make arduino IDE happy about the TurntableState
void changeState(TurntableState newState); 

//replace with counter
uint16_t  armPosition = 9999;
uint16_t  desiredPosition = 0;
bool armPositionInitialized = false;

//output to arm lifter
bool armLifter = false;

//replace with actual switch
bool armReset = true;

//outputs to DCM
bool DCM1 = false;
bool DCM2 = false;
bool DCM3 = false;

//outputs to DD
bool DDSS = false;
bool DD30 = false;
bool DD45 = false;

//todo: replace these with actual sensors
bool sense30 = true;
bool sense45 = true;

bool isHome() {
  armPosition == positionHome;
}

bool isOverPlatter() {
  armPosition >= positionTo30;
}

void changeState(TurntableState newState) {
  if(highVerbosity) Serial.println(String("State: ") + TurntableStateDesc[currentState] + " -> " + TurntableStateDesc[newState]);
  currentState = newState;
}

uint16_t  getArmPosition() {
  //todo: link with counter
  return armPosition;
}

bool isArmUp() {
  return armLifter == armUp;
}

bool isArmDown() {
  return armLifter == armDown;
}

void raiseArm() {
  armLifter = armUp;
}

void lowerArm() {
  if (isOverPlatter()) armLifter = armDown;
}

void toggleArm() {
  if (armLifter == armUp) lowerArm();
  else raiseArm();
}

bool isArmReady() {
  return armPositionInitialized && armLifter == armUp;
}

void resetArmposition() {
  armPosition = 0;
  armPositionInitialized = true;
}

void setDCM(int DCMNumber) {
  DCM1 = (DCMNumber == 1);
  DCM2 = (DCMNumber == 2);
  DCM3 = (DCMNumber == 3);
}

void moveArmRight() {
  raiseArm();
  setDCM(3);
  //temp debug
  if (millis() % 100 == 0) armPosition--;
}

void moveArmLeft() {
  raiseArm();
  setDCM(1);
  //temp debug
  if (millis() % 100 == 0) armPosition++;
}

void enableServo() {
  setDCM(2);
}

void moveArmTo(uint16_t  position) {
  if (armPosition > position) 
  {
  	moveArmRight();
  }
  else if (armPosition < position) 
  {
    moveArmLeft();
  }
}

bool reachedArmReset() {
  //todo: this needs to use the physical switch
  return armPosition == 0;
}

bool reachedHome() {
  //todo: this needs to use the physical switch
  return armPosition == 0;
}

bool reachedPosition() {
  return armPosition == desiredPosition;
}

void startDD() {
  DDSS = true;
}

void stopDD() {
  DDSS = false;
}

bool isTurning() {
  return DDSS == true;
}

void playRecord() {
  lowerArm();
  enableServo();
}

//Tonearm control=============================
void turntableSetup() {
  changeState(INITIAL);
}

void turntableReport() {
  Serial.println("==========REPORT============");
  Serial.print("status: "); Serial.println(TurntableStateDesc[currentState]);
  Serial.print("armPosition: "); Serial.println(armPosition);
  Serial.print("desired: "); Serial.println(desiredPosition);
  Serial.print("armInitialized: "); Serial.println(armPositionInitialized ? "YES" : "NO");
  Serial.print("armLifter: "); Serial.println(armLifter ? "UP" : "DOWN");
  Serial.print("armReset: "); Serial.println(armReset ? "YES" : "NO");

  Serial.print("DCM    :");
  Serial.print("  1-"); Serial.print (DCM1 ? "HI" : "LO");
  Serial.print("  2-"); Serial.print (DCM2 ? "HI" : "LO");
  Serial.print("  3-"); Serial.println(DCM3 ? "HI" : "LO");

  Serial.print("DD     :");
  Serial.print("  SS-"); Serial.print (DDSS ? "HI" : "LO");
  Serial.print("  30-"); Serial.print (DD30 ? "HI" : "LO");
  Serial.print("  45-"); Serial.println(DD45 ? "HI" : "LO");

  Serial.print("Sensors:");
  Serial.print("  30-"); Serial.print (sense30 ? "HI" : "LO");
  Serial.print("  45-"); Serial.println(sense45 ? "HI" : "LO");
  Serial.println("============================");
}

//for UI
String turntableStatus() {
 return TurntableStateDesc[currentState];
}


//the requests are for user interface & callbacks to interact
void requestPlayStop() {
  if (currentState == PLAY) requestHome();
  else changeState(PLAY);
}

void requestHome() {
  nextState = IDLE;
  changeState(GOHOME);
}

void requestUpDown() {
  toggleArm();
}

void requestMove(uint16_t  delta) {
  desiredPosition = armPosition + delta;
  nextState = IDLE;
  changeState(MOVE);
}

void turntableLoop() {
  switch (currentState) {
    case IDLE:
      if (isHome())
      {
        stopDD();
        lowerArm();
      } else
      {
        raiseArm();
        startDD();
      }
      //handle buttons
      break;
    case INITIAL:
      moveArmRight();
      if (reachedArmReset()) changeState(IDLE);
      //do not handle buttons
      break;
    case GOHOME:
      moveArmTo(positionHome);
      if (reachedHome()) changeState(IDLE);
      //handle buttons
      break;
    case MOVE:
      //validate platter
      moveArmTo(desiredPosition);
      if (reachedPosition()) changeState(nextState);
      //handle buttons
      break;
    case DETECT:
      raiseArm();
      startDD();
      //detect record size
      //this takes a little time
      //set record speed and change state
      break;
    case PLAY:
      if (isOverPlatter() && isTurning()) playRecord();
      else changeState(DETECT);
      break;
    }
  //handle sensors
  //arm reset
  if (armPosition == 0) resetArmposition();
  //15 & 30cm sensors
  //buttons are handled in the UI
}
//Tonearm control=============================
