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
const uint16_t  positionTo30 = 50; //todo: determine
const uint16_t  positionTo15 = 100; //todo: determine
const uint16_t  endPosition = 9000; //todo: determine

enum TurntableState {IDLE, INITIAL, REPEAT, GOHOME, MOVE, DETECT, PLAY};
const char* TurntableStateDesc[] = {"Idle", "Initialization", "Repeating", "Going home", "Arm in motion", "Detection", "Playing"};
TurntableState currentState = IDLE;
TurntableState nextState = IDLE;
//prototype to make arduino IDE happy about the TurntableState
void changeState(TurntableState newState); 
bool isState(TurntableState state);

//replace with counter
uint16_t  armPosition = 7500; //temp debug value to have a fake "intiialization"
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
  if(highVerbosity) webSerialPrintln(String(millis()) + " - State: " + TurntableStateDesc[currentState] + " -> " + TurntableStateDesc[newState]);
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
  if(highVerbosity && !armAlreadyReset) webSerialPrintln(String(millis()) + " - Resetting armPosition");
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
  DDSS = DdActive;
}

void stopDD() {
  DDSS = DdInactive;
}

bool isTurning() {
  return DDSS == DdActive;
}

void playRecord() {
  lowerArm();
  enableServo();
}

//Tonearm control=============================
void turntableSetup() {
  webSerialPrintln(String(millis()) + " - Starting initialization sequence");
  changeState(INITIAL);
}

void turntableReport() {
  webSerialPrintln("===========REPORT============");
  webSerialPrintln("timestamp:       " + millis());
  webSerialPrintln(String("status:          ") + TurntableStateDesc[currentState]);
  webSerialPrintln(String("Next state:      ") + TurntableStateDesc[nextState]);
  webSerialPrintln(String("armPosition:     ") + armPosition + " (" + desiredPosition + ")");
  webSerialPrintln(String("Initialization:  ") + (initializationCompleted ? "Completed" : "Pending"));
  webSerialPrintln(String("arm lifter:      ") + (armLifter == armUp ? "armUp" : "armDown"));
  webSerialPrintln(String("armReset switch: ") + (armReset == pressed ? "Pressed" : "Released"));

  webSerialPrint("DCM    :");
  webSerialPrint  ((String("   1-")) + (DCM1 == DcmActive ? "ON" : "--"));
  webSerialPrint  ((String("   2-")) + (DCM2 == DcmActive ? "ON" : "--"));
  webSerialPrintln((String("   3-")) + (DCM3 == DcmActive ? "ON" : "--"));

  webSerialPrint("DD     :");
  webSerialPrint  ((String("  SS-")) + (DDSS == DdActive ? "EN" : "--"));
  webSerialPrint  ((String("  30-")) + (DD30 == DdActive ? "EN" : "--"));
  webSerialPrintln((String("  45-")) + (DD45 == DdActive ? "EN" : "--"));

  webSerialPrint("Sensors:");  //todo: add logic to enable proper sensing
  webSerialPrint  ((String("  30-")) + (sense30 ? "HI" : "LO"));
  webSerialPrintln((String("  45-")) + (sense45 ? "HI" : "LO"));

  webSerialPrint("Various:"); //other data?
  webSerialPrintln((String("  Repeat-")) + (repeat ? "YES" : "NO"));
  webSerialPrintln("=============================");
}

//for UI
String turntableStatus() {
 return TurntableStateDesc[currentState];
}

void returnAndClear() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - Return (and clear)");
  changeState(GOHOME);
  repeat = false;
}

void startAutoOperation() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - Start Operation");
  changeState(DETECT);
}

void clearRepeat() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - Clear repeat");  
  repeat = false;
}


//the requests are for user interface & callbacks to interact
void requestPlayStop() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Play/Stop");
  if (!initializationCompleted) return;

  if(highVerbosity)
  {
    webSerialPrintln("| Lifter |ArmReset |  DDSS  |"); 
    webSerialPrintln("| (p31)  | (p25)   | (p38)  |"); 
    webSerialPrint  (armLifter == armDown ? "| DN (H) " : "| UP (L) ");
    webSerialPrint  (armReset == pressed ? "| HOME(L) " : "| away(H) ");
    webSerialPrintln(DDSS == DdInactive ? "| OFF(H) |" : "| ON (L) |");
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
       if (armLifter == armUp   && armReset == pressed  && DDSS == DdInactive) startAutoOperation();
  else if (armLifter == armUp   && armReset == released && DDSS == DdInactive) startDD();
  else if (armLifter == armDown && armReset == released && DDSS == DdInactive) startDD();
  else if (armLifter == armUp   && armReset == released && DDSS == DdActive) returnAndClear();
  else if (armLifter == armDown && armReset == released && DDSS == DdActive) returnAndClear();
  else if (isState(DETECT)) returnAndClear();
  else if (isState(GOHOME)) clearRepeat();
}

void requestHome() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request HOME");
  if (!initializationCompleted) return;
  nextState = IDLE;
  changeState(GOHOME);
}

void requestGoEnd() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request GoEND");
  if (!initializationCompleted) return;
  nextState = IDLE;
  requestMove(endPosition);
}

void requestGo30() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Go30");
  if (!initializationCompleted) return;
  nextState = IDLE;
  requestMove(positionTo30);
}

void requestGo15() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Go15");
  if (!initializationCompleted) return;
  nextState = IDLE;
  requestMove(positionTo15);
}

void requestGoStill() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Still");
  if (!initializationCompleted) return;
  nextState = IDLE;
  requestMove(armPosition);
}

void requestUpDown() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request UP/DOWN");
  if (!initializationCompleted) return;
       if (armLifter == released && !isHome()) changeState(PLAY);
  else if (armLifter == pressed && !isHome()) changeState(IDLE);
}

void requestMoveIn(uint16_t  delta) {
  if (armPosition > endPosition - delta) return; //no rollover
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Move IN");
  if( !isAtEndPosition() && armLifter == released) requestMove(armPosition + delta);
}

void requestMoveOut(uint16_t  delta) {
  if (armPosition < delta) return; //no rollover
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Move OUT");
  if( !isHome() && armLifter == released) requestMove(armPosition - delta);
}

void requestMove(uint16_t  newPosition) {
  if (!initializationCompleted) return;
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Move to " + newPosition);
  desiredPosition = newPosition;
  nextState = IDLE;
  changeState(MOVE);
}

void requestRepeat() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Repeat");
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
    case REPEAT:
      raiseArm();
      nextState = PLAY;
      //todo: detect record size
      desiredPosition = positionTo30;
      moveArmTo(desiredPosition);//temp: put "detected record size"
      if (reachedPosition()) changeState(nextState);
      break;
    case GOHOME:
      nextState = IDLE;
      desiredPosition = positionHome;
      moveArmTo(desiredPosition);
      if (reachedHome()) changeState(nextState);
      break;
    case MOVE:
      moveArmTo(desiredPosition);
      if (reachedPosition()) changeState(nextState);
      break;
    case DETECT:
      startDD();
      nextState = PLAY;
      //todo: detect record size
      desiredPosition = positionTo30;
      moveArmTo(desiredPosition);//temp: put "detected record size"
      //this takes a little time
      //set record speed and change state
      break;
    case PLAY:
      if (isOverPlatter() && isTurning()) playRecord();
      else changeState(DETECT);
      if (armPosition >= endPosition) {
        if (repeat) changeState(REPEAT); else changeState(GOHOME);
      }
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
