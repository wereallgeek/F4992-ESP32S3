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

String turntableReport() {
  String report = "";
  report += String("===============REPORT===============\n");
  report += String("status: ") + turntableStatus() + "\n";
  report += String("armPosition: ") + armPosition + "\n";
  report += String("desiredPosition: ") + desiredPosition + "\n";
  report += String("armPositionInitialized: ") + (armPositionInitialized ? "YES" : "NO") + "\n";
  report += String("armLifter: ") + (armLifter ? "UP" : "DOWN") + "\n";
  report += String("armReset: ") + (armReset ? "YES" : "NO") + "\n";

  report += String("DCM1: ") + (DCM1 ? "HI" : "LO");
  report += String(" DCM2: ") + (DCM2 ? "HI" : "LO");
  report += String(" DCM3: ") + (DCM3 ? "HI" : "LO") + "\n";

  report += String("DDSS: ") + (DDSS ? "HI" : "LO");
  report += String(" DD30: ") + (DD30 ? "HI" : "LO");
  report += String(" DD45: ") + (DD45 ? "HI" : "LO") + "\n";
  report += String("sense30: ") + (sense30 ? "HI" : "LO");
  report += String(" sense45: ") + (sense45 ? "HI" : "LO") + "\n\n";
  return report;
}


String turntableStatus() {
 switch (currentState) {
    case IDLE:
      return "Idle";
      break;
    case INITIAL:
      return "Initialization";
      break;
    case GOHOME:
      return "Going home";
      break;
    case MOVE:
      return "Arm in motion";
      break;
    case DETECT:
      return "Detection";
      break;
    case PLAY:
      return "Playing";
      break;
    }
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
