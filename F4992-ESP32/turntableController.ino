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

const bool released = false;
const bool pressed = true;

const bool DdActive = false;
const bool DdInactive = true;

const bool DcmActive = true;
const bool DcmInactive = false;

const uint16_t  positionHome = 0;
const uint16_t  positionTo30 = 200; //todo: determine
const uint16_t  positionTo15 = 400; //todo: determine
const uint16_t  endPosition = 4000; //todo: determine

enum TurntableState {IDLE, INITIAL, GOHOME, MOVE, DETECT, PLAY};
const char* TurntableStateDesc[] = {"Idle", "Initialization", "Going home", "Arm in motion", "Detection", "Playing"};
TurntableState currentState = IDLE;
TurntableState nextState = IDLE;
//prototype to make arduino IDE happy about the TurntableState
void changeState(TurntableState newState); 
bool isState(TurntableState state);

//replace with counter
uint16_t  armPosition = 2500; //temp debug value to have a fake "intiialization"
uint16_t  desiredPosition = 0;
bool initializationCompleted = false;

//output to arm lifter
bool armLifter = released;


//outputs to DCM
bool DCM1 = DcmInactive;
bool DCM2 = DcmInactive;
bool DCM3 = DcmInactive;

//outputs to DD
bool DDSS = DdInactive;
bool DD30 = DdInactive;
bool DD45 = DdInactive;

//todo: replace with actual switch
bool armReset = released;
//todo: replace these with actual sensors
bool sense30 = true;
bool sense45 = true;

bool repeat = false;

//for debug
bool armAlreadyReset = false;

bool isHome() {
  return armPosition == positionHome;
}

bool isOverPlatter() {
  return armPosition >= positionTo30;
}

bool isAtEndPosition() {
  return armPosition >= endPosition;
}

void changeState(TurntableState newState) {
  if(highVerbosity) Serial.println(String(millis()) + " - State: " + TurntableStateDesc[currentState] + " -> " + TurntableStateDesc[newState]);
  currentState = newState;
}

bool isState(TurntableState state) {
  return currentState == state;
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
  return initializationCompleted && armLifter == armUp;
}

void resetArmposition() {
  initializationCompleted = true;
  if(highVerbosity && !armAlreadyReset) Serial.println(String(millis()) + " - Resetting armPosition");
  armPosition = 0;
  initializationCompleted = true;
  armAlreadyReset = true;
}

void setDCM(int DCMNumber) {  
  DCM1 = (DCMNumber == 1) ? DcmActive : DcmInactive;
  DCM2 = (DCMNumber == 2) ? DcmActive : DcmInactive;
  DCM3 = (DCMNumber == 3) ? DcmActive : DcmInactive;
}

void moveArmRight() {
  raiseArm();
  setDCM(3);
  //temp debug
  if (millis() % 100 == 0 && armPosition>0) armPosition--;//temp debug
}

void moveArmLeft() {
  raiseArm();
  setDCM(1);
  //temp debug
  if (millis() % 100 == 0) armPosition++;//temp debug
}

void enableServo() {
  setDCM(2);
}

void dontMove()
{
  setDCM(0);
}


void moveArmTo(uint16_t  position) {
  if (armPosition > position) {
  	moveArmRight();
  }
  else if (armPosition < position) {
    moveArmLeft();
  }
  else {
    dontMove();
  }
}

bool reachedArmReset() {
  //todo: this needs to use the physical switch
  return armReset == pressed;
}

bool reachedHome() {
  return armPosition == 0;
}

bool reachedPosition() {
  return armPosition == desiredPosition;
}

bool reachedEndPosition() {
  return armPosition >= endPosition;
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
  Serial.println(String(millis()) + " - Waking up [" + stored_devicename + "]");
  changeState(INITIAL);
}

void turntableReport() {
  Serial.println("===========REPORT============");
  Serial.print("timestamp:     "); Serial.println(millis());
  Serial.print("status:        "); Serial.println(TurntableStateDesc[currentState]);
  Serial.print("armPosition:   "); Serial.println(armPosition);
  Serial.print("desired:       "); Serial.println(desiredPosition);
  Serial.print("Init complete: "); Serial.println(initializationCompleted ? "YES" : "NO");
  Serial.print("armLifter:     "); Serial.println(armLifter  == pressed ? "pressed" : "released");
  Serial.print("armReset:      "); Serial.println(armReset == pressed ? "pressed" : "released");

  Serial.print("DCM    :");
  Serial.print("   1-"); Serial.print (DCM1 ? "HI" : "LO");
  Serial.print("   2-"); Serial.print (DCM2 ? "HI" : "LO");
  Serial.print("   3-"); Serial.println(DCM3 ? "HI" : "LO");

  Serial.print("DD     :");
  Serial.print("  SS-"); Serial.print (DDSS ? "HI" : "LO");
  Serial.print("  30-"); Serial.print (DD30 ? "HI" : "LO");
  Serial.print("  45-"); Serial.println(DD45 ? "HI" : "LO");

  Serial.print("Sensors:");
  Serial.print("  30-"); Serial.print (sense30 ? "HI" : "LO");
  Serial.print("  45-"); Serial.println(sense45 ? "HI" : "LO");
  Serial.println("=============================");
}

//for UI
String turntableStatus() {
 return TurntableStateDesc[currentState];
}

void returnAndClear() {
  if(highVerbosity) Serial.println(String(millis()) + " - Return (and clear)");
  changeState(GOHOME);
  repeat = false;
}

void startAutoOperation() {
  if(highVerbosity) Serial.println(String(millis()) + " - Start Operation");
  changeState(DETECT);
}

void clearRepeat() {
  if(highVerbosity) Serial.println(String(millis()) + " - Clear repeat");  
  repeat = false;
}


//the requests are for user interface & callbacks to interact
void requestPlayStop() {
  if(highVerbosity) Serial.println(String(millis()) + " - request Play/Stop");
  if (!initializationCompleted) return;

  if(highVerbosity)
  {
    Serial.print(String(millis()));
    Serial.print(" - L(31)="); Serial.print(armLifter ? "HI" : "LO");
    Serial.print(" | A(25)="); Serial.print(armReset ? "HI" : "LO");
    Serial.print(" | D(38)="); Serial.println(DDSS ? "HI" : "LO");
  }
  // section 2-4 key operation
  // Start/Stop (pin24) key
  //|---------------------------|--------------------------|
  //|        Condition          |          Result          |
  //|---------------------------|--------------------------|
  //| Lifter |ArmReset |  DDSS  | Auto-  |  DDSS  | repeat |
  //| (p31)  | (p25)   | (p38)  | operat |  (p38) |        |
  //|---------------------------|--------------------------|
  //| UP (L) | HOME(L) | OFF(H) |    START        |   ---  |
  //|---------------------------|--------------------------|
  //| UP (L) | away(H) | OFF(H) |  ---   | ON (L) |   ---  |
  //|---------------------------|--------------------------|
  //| DN (H) | away(H) | OFF(H) |  ---   | ON (L) |   ---  |
  //|---------------------------|--------------------------|
  //| UP (L)*| away(H) | ON (L) |      RETURN     |  Clear |
  //|---------------------------|--------------------------|
  //| DN (H) | away(H) | ON (L) |      RETURN     |  Clear |
  //|---------------------------|--------------------------|
  //|During autostart operation |      RETURN     |  Clear |
  //|---------------------------|--------------------------|
  //|During autoreturn operation|        ---      |  Clear |
  //|---------------------------|--------------------------|
  // * return after 2.5s via the key operation during the UP using the UP/DOWN key
       if (!armLifter && !armReset &&  DDSS) startAutoOperation();
  else if (!armLifter &&  armReset &&  DDSS) startDD();
  else if ( armLifter &&  armReset &&  DDSS) startDD();
  else if (!armLifter &&  armReset && !DDSS) returnAndClear();
  else if ( armLifter &&  armReset && !DDSS) returnAndClear();
  else if (isState(DETECT)) returnAndClear();
  else if (isState(GOHOME)) clearRepeat();
}

void requestHome() {
  if(highVerbosity) Serial.println(String(millis()) + " - request HOME");
  if (!initializationCompleted) return;
  nextState = IDLE;
  changeState(GOHOME);
}

void requestUpDown() {
  if(highVerbosity) Serial.println(String(millis()) + " - request UP/DOWN");
  if (!initializationCompleted) return;
       if (armLifter = released && !isHome()) changeState(PLAY);
  else if (armLifter = pressed && !isHome()) changeState(IDLE);
}

void requestMoveIn(uint16_t  delta) {
  if(highVerbosity) Serial.println(String(millis()) + " - request Move IN");
  if( !isAtEndPosition() && armLifter == released) requestMove(armPosition + delta);
}

void requestMoveOut(uint16_t  delta) {
  if(highVerbosity) Serial.println(String(millis()) + " - request Move OUT");
  if( !isHome() && armLifter == released) requestMove(armPosition - delta);
}

void requestMove(uint16_t  newPosition) {
  if (!initializationCompleted) return;
  if(highVerbosity) Serial.println(String(millis()) + " - request Move to " + newPosition);
  desiredPosition = newPosition;
  nextState = IDLE;
  changeState(MOVE);
}

void requestRepeat() {
  if(highVerbosity) Serial.println(String(millis()) + " - request Repeat");
  repeat = !repeat;
}

void turntableLoop() {
  switch (currentState) {
    case IDLE:
      if (isHome()) {
        stopDD();
      } 
      else {
        raiseArm();
        startDD();
      }
      dontMove();
      break;
    case INITIAL:
      moveArmRight();
      if (reachedArmReset()) changeState(IDLE);
      break;
    case GOHOME:
      moveArmTo(positionHome);
      if (reachedHome()) changeState(IDLE);
      break;
    case MOVE:
      moveArmTo(desiredPosition);
      if (reachedPosition()) changeState(nextState);
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
  //read arm reset
  armReset = (armPosition == 0) ? pressed : released;//temp debug reading
  //process armReset
  if (armReset == pressed) resetArmposition(); else armAlreadyReset = false;
  //15 & 30cm sensors
  //buttons are handled in the UI
}
//Tonearm control=============================
