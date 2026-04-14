//================================================
// Turntable Controller
//
// Handles hardware automation 
//    for full tonearm and platter control
//
//================================================
unsigned long lastUpdateMillis = 0; 

//todo: define appropriately for correct logic-level
const bool armUp = false;
const bool armDown = true;

const bool released = false;
const bool pressed = true;

const bool DdActive = false;
const bool DdInactive = true;

const bool DcmActive = true;
const bool DcmInactive = false;

//purposely match arm position and speed
enum DetectedSize {NODISC, DISC30, DISC15};
const char* sizename[] = {"NODISC", "30cm", "15cm"};
DetectedSize DiscSize = NODISC;
DetectedSize previousDiscSize = NODISC;
enum ArmPosition {HOME, START30, START15, END};
const uint16_t Steps[] = {0, 55, 110, 7500};//todo: determine 30, 15, end
//replace with counter
uint16_t  armPosition = Steps[END]; //Steps[HOME];temp debug value to have a fake "intitalization"
uint16_t  desiredPosition = Steps[HOME];

enum TurntableState {IDLE, INITIAL, GOHOME, MOVE, DETECT, PLAY};
const char* TurntableStateDesc[] = {"Idle", "Initialization", "Going home", "Arm in motion", "Detection", "Playing"};
const ControlColor statusColor[] = {Dark, Sunflower, Carrot, Carrot, Sunflower, Peterriver};
const char* statusHexColor[]     = {"#2c3e50", "#f1c40f", "#e67e22", "#e67e22", "#f1c40f", "#3498db"};

TurntableState currentState = IDLE;
TurntableState nextState = IDLE;
//prototype to make arduino IDE happy about the TurntableState
void changeState(TurntableState newState); 
bool isState(TurntableState state);

bool initializationCompleted = false;

//output to arm lifter
bool armLifter = released;


//outputs to DCM
bool DCM1 = DcmInactive;
bool DCM2 = DcmInactive;
bool DCM3 = DcmInactive;

//outputs to DD
bool DDSS = DdInactive;
bool DD33 = DdInactive;
bool DD45 = DdInactive;

//todo: replace with actual switch
bool armReset = released;
//todo: replace these with actual sensors
bool sense30 = true;
bool sense15 = true;
unsigned long sensortimer = 0;
unsigned long  DetectionTime[3] = {0, 0, 0};

bool repeat = false;

//for debug
bool armAlreadyReset = false;

bool isHome() {
  return armPosition == Steps[HOME];
}

bool isOverPlatter() {
  return armPosition >= Steps[START30];
}

bool isAtEndPosition() {
  return armPosition >= Steps[END];
}

void changeState(TurntableState newState) {
  if(firstPassCompleted && highVerbosity) webSerialPrint(String(millis()) + " - "); //state change remains in low verbosity
  if(firstPassCompleted) webSerialPrintln(String("State: ") + TurntableStateDesc[currentState] + " -> " + TurntableStateDesc[newState]);
  currentState = newState;
}

bool isState(TurntableState state) {
  return currentState == state;
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

void computeAutoSpeed() {
  if (DetectionTime[DISC30] > millis() - 2500) DiscSize = DISC30;
  else if (DetectionTime[DISC15] > millis() - 2500) DiscSize = DISC15;
  else DiscSize = NODISC;
  if (highVerbosity && previousDiscSize != DiscSize)  webSerialPrintln(String(millis()) + " - Detected " + sizename[DiscSize]);
  previousDiscSize = DiscSize;
  setAutoDDspeed();
}

bool discPresent() {
  return DiscSize != NODISC;
}

void setAutoDDspeed() {
  DD45 = ((DiscSize == DISC15) ^ softSpeedInverter) ? DdActive : DdInactive; //if nodisc - 33rpm
  DD33 = ((DiscSize == DISC30 || DiscSize == NODISC) ^ softSpeedInverter) ? DdActive : DdInactive;
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
  return armPosition >= Steps[END];
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
  webSerialPrintln(String("Starting initialization sequence\nTimestamp: ") + millis());
  changeState(INITIAL);
  setAutoDDspeed();
}

void returnAndClear() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - Return (and clear)");
  changeState(GOHOME);
  repeat = false;
}

void startAutoOperation() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - Start Operation");
  sensortimer = millis();
  changeState(DETECT);
}

void clearRepeat() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - Clear repeat");  
  repeat = false;
}

//Tonearm control=============================

// Turntable user interface ==============================================
void turntableReport() {
  webSerialPrintln("================REPORT=================");
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
  webSerialPrint  ((String("  33-")) + (DD33 == DdActive ? "EN" : "--"));
  webSerialPrintln((String("  45-")) + (DD45 == DdActive ? "EN" : "--"));

  webSerialPrint("Sensors:");  //todo: add logic to enable proper sensing
  webSerialPrint  ((String("  30-")) + (sense30 ? "HI(" : "LO(") + DetectionTime[DISC30] + ")");
  webSerialPrintln((String("  15-")) + (sense15 ? "HI(" : "LO(") + DetectionTime[DISC15] + ")");

  webSerialPrintln((String(sizename[DiscSize])) + (softSpeedInverter ? " | -INVERT-" : " | noinvert") + (repeat ? " | -REPEAT-" : " | norepeat"));
  webSerialPrintln("=======================================");
}

//for UI
String turntableStatus() {
 return TurntableStateDesc[currentState];
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
  requestMove(Steps[END]);
}

void requestGo30() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Go30");
  if (!initializationCompleted) return;
  nextState = IDLE;
  requestMove(Steps[START30]);
}

void requestGo15() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Go15");
  if (!initializationCompleted) return;
  nextState = IDLE;
  requestMove(Steps[START15]);
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
  else if (armLifter == pressed && !isHome())  changeState(IDLE);
}

void requestMoveIn(uint16_t  delta) {
  if (armPosition > Steps[END] - delta) return; //no rollover
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Move IN");
  if( !isAtEndPosition() && armLifter == released) requestMove(armPosition + delta);
}

void requestMoveOut(uint16_t  delta) {
  if (armPosition < delta) return; //no rollover
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Move OUT");
  if( !isHome() && armLifter == released) requestMove(armPosition - delta);
}

void requestMove(uint16_t  newPosition) {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Move to " + newPosition);
  if (!initializationCompleted) return;
  desiredPosition = newPosition;
  nextState = IDLE;
  changeState(MOVE);
}

void requestRepeat() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Repeat");
  repeat = !repeat;
}

void requestInvert(bool invertRequested) {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request " + (invertRequested ? "[INV] speed" : "[NOR] speed"));
  softSpeedInverter = invertRequested;
}

String armPositionStatus(uint16_t position) {
  char buffer[25];
  snprintf(buffer, sizeof(buffer), "Arm position: %u", position);
  return String(buffer);
}

void changeEspuiPanelColor(uint16_t id, ControlColor newColor) {
    Control* panel = ESPUI.getControl(id);
    if (panel != nullptr) {
      panel->color = newColor;
      ESPUI.updateControl(id);
    }
}

void changeEspuiIndicatorColor(uint16_t id, const char* colorHex) {
    String circleStyle = "background-color: " + String(colorHex) + espuiIndicatorElementStyle;

    ESPUI.setElementStyle(id, circleStyle.c_str());
    ESPUI.updateControl(id);
}

void turntableUiUpdate() {
  // conditionnal ui update
  if (millis() - lastUpdateMillis >= 750) {
    lastUpdateMillis = millis();
    ESPUI.print(armStatusLabelId, turntableStatus());
    changeEspuiPanelColor(armStatusLabelId, statusColor[currentState]);
    changeEspuiIndicatorColor(ledId, statusHexColor[currentState]);    
    ESPUI.print(armPositionLabelId, armPositionStatus(armPosition));
    updateWebSerial();
  }
}
// Turntable user interface ==============================================


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
      nextState = IDLE;
      desiredPosition = Steps[HOME];
      moveArmTo(desiredPosition);
      if (reachedHome()) changeState(nextState);
      break;
    case MOVE:
      setAutoDDspeed();
      moveArmTo(desiredPosition);
      if (reachedPosition()) changeState(nextState);
      break;
    case DETECT:
      startDD();
      setAutoDDspeed();
      nextState = DETECT;
      desiredPosition = Steps[DiscSize];
      moveArmTo(Steps[DISC30]); // begin arm movement to larger disc
      //section 2 automatic disk selection timing says Input for 2.5 sec
      if (millis() - sensortimer >= 2500) {
        nextState = PLAY;
        moveArmTo(desiredPosition);
        changeState(MOVE); 
      }
      //set record speed and change state
      break;
    case PLAY:
      setAutoDDspeed();
      if (isOverPlatter() && isTurning()) playRecord();
      if (!discPresent()) changeState(GOHOME);
      else if (armPosition >= Steps[END]) {
        if (repeat) {
          nextState = PLAY;
          desiredPosition = Steps[DiscSize];
          repeat = false; //no infinite repeat
          changeState(MOVE); 
        }
        else {
          changeState(GOHOME);
        }
      }
      break;
    }
  //handle sensors
  //read arm reset
  armReset = (armPosition == 0) ? pressed : released;//temp debug reading
  //process armReset
  if (armReset == pressed) resetArmposition(); else armAlreadyReset = false;
  //15 & 30cm sensors -- may use hardware interrupt
  if (sense30) DetectionTime[DISC30] = millis();//todo: use actual sensor
  if (sense15) DetectionTime[DISC15] = millis();//todo: use actual sensor
  if (isTurning()) computeAutoSpeed();

  turntableUiUpdate();
}


//DEBUG
void toggleSensor15() {
  sense15 = !sense15;
}
void toggleSensor30() {
  sense30 = !sense30;
}