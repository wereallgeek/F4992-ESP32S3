//================================================
// Turntable Controller
//
// Handles hardware automation 
//    for full tonearm and platter control
//
//================================================
#include <Bounce2.h>
#include "driver/pulse_cnt.h"

unsigned long lastUpdateCycle1 = 0; 
unsigned long lastUpdateCycle2 = 0; 
unsigned long lastUpdateCycle3 = 0; 
unsigned long lastUpdateCycle4 = 0; 

//============================ hardware interface =============================
//here declaration for the hardware pins.

// ARM POSITION COUNTER
#define PIN_COUNTER          6   

//Infrared Sensors
#define PIN_IR30             1
#define PIN_IR17             21

// Switches (Inputs)
#define PIN_SW1              5
#define PIN_SW2              7
#define PIN_SW3              8
#define PIN_SW4              9
#define PIN_SW5              10
#define PIN_ARMRESET         16

// DCM Arm tray (Outputs)
#define PIN_DCM1             17  
#define PIN_DCM2             14
#define PIN_DCM3             4   
#define PIN_LIFTER           13
#define PIN_MUTING           18  

// Direct Drive module
#define PIN_DDSS             11
#define PIN_DD33             15
#define PIN_DD45             2

// LEDs (Outputs)
#define PIN_LED1             39
#define PIN_LED2             40
#define PIN_LED3             42
#define PIN_LED4             41

//compuselector. 
#define PIN_COMPUSELECTOR    12


//todo: properly define
#define DETECTIONDURATION    2500
#define MUTEPOSTDOWN         1000

enum Hardwareswitch          {ARM,          SWITCH1,  SWITCH2,    SWITCH3,    SWITCH4,   SWITCH5, MAXSWITCH};
const char* Switchname[] =   {"ArmReset",   "Repeat", "Move In",  "Move Out", "Up/Down", "Start/Stop"};
const byte switchpins[] =    {PIN_ARMRESET, PIN_SW1,  PIN_SW2,    PIN_SW3,    PIN_SW4,    PIN_SW5};
Bounce debouncedButtons[MAXSWITCH]; 

//align hardware number with logical number and add minimum and maximum values.
enum Hardwareled             {NOLED,        LED1,         LED2,      LED3,      LED4,       MAXLED};
const byte ledpins[] =       {0,            PIN_LED1,     PIN_LED2,  PIN_LED3,  PIN_LED4};

enum Hardwaredcm             {NODCM,        DCM1,         DCM2,      DCM3,      MAXDCM};
const byte dcmpins[] =       {0,            PIN_DCM1,     PIN_DCM2,  PIN_DCM3};
//============================ state machine logic ============================

const int armUp =        LOW;
const int armDown =      HIGH;

const int mutesound =    LOW;
const int playsound =    HIGH;

const int released =     HIGH;
const int pressed =      LOW;

const int DdActive =     LOW;
const int DdInactive =   HIGH;

const bool DcmActive =   HIGH;
const bool DcmInactive = LOW;

const bool IN =          true;
const bool OUT =         false;

const int irNotvitible = HIGH;
const int irVisible =    LOW;

const int bStop =        HIGH;
const int bStart =        LOW;

//purposely match arm position and speed
enum DetectedSize               {NODISC, DISC30, DISC17};
const char* sizename[] =        {"NODISC", "30cm", "17cm"};
const char* recordStyles[] =    { recordNodiscStyle, record33style, record45style };
DetectedSize DiscSize =         NODISC;
DetectedSize previousDiscSize = NODISC;
enum ArmPositions               {HOME, START30, START17, END};
const uint16_t Steps[] =        {0,    55,      110,    7500};//todo: determine 30, 17, end
//replace with counter
uint16_t  desiredPosition =     Steps[HOME];

enum TurntableState                {IDLE,      INITIAL,        GOHOME,      MOVE,      DETECT,      PLAY};
const char* TurntableStateDesc[] = {"Idle",    "Initializing", "Returning", "Moving",  "Detecting", "Playing"};
const ControlColor statusColor[] = {Dark,      Sunflower,      Carrot,      Carrot,    Sunflower,   Peterriver};
const char* statusHexColor[] =     {"#2c3e50", "#f1c40f",      "#e67e22",   "#e67e22", "#f1c40f",   "#3498db"};

TurntableState currentState =      IDLE;
TurntableState nextState =         IDLE;

const char* onOffIndicatorColor[] =     {"#2c3e50", "#e67e22"};

//prototype to make arduino IDE happy about the TurntableState
void changeState(TurntableState newState); 
bool isState(TurntableState state);

bool initializationCompleted = false;

const char* armIcons[] = {"\xE2\xA4\x93", "\xE2\xA8\xAA"}; 
const char* dcmIcons[] = {"\xF0\x9F\x92\xA4", "\xE2\x87\x90", "\xE2\x8E\x8C", "\xE2\x87\x92"};

unsigned long sensortimer = 0;
unsigned long DetectionTime[3] = {0, 0, 0};
unsigned long armdowntime = 0;

bool repeat = false;

pcnt_unit_handle_t counterUnit = NULL;
pcnt_channel_handle_t counterChan = NULL;

//============================ hardware interface =============================
void turntableCounterSetup() {
  //counter unit
  pcnt_unit_config_t counterUnitCfg = {
      .low_limit = -32768,
      .high_limit = 32767,
  };
  pcnt_new_unit(&counterUnitCfg, &counterUnit);

  //canal 
  pcnt_chan_config_t  counterChanCfg = {
    .edge_gpio_num = PIN_COUNTER,
    .level_gpio_num = -1,
    .flags = { .io_loop_back = 0 },
  };
  pcnt_new_channel(counterUnit, &counterChanCfg, &counterChan);

  //action
  pcnt_channel_set_edge_action(counterChan, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);

  //start
  pcnt_unit_enable(counterUnit);
  pcnt_unit_clear_count(counterUnit);
  pcnt_unit_start(counterUnit);
}

void turntableSwitchSetup () {
  // switch INPUTS
  for (int pinnumber = ARM; pinnumber < MAXSWITCH; pinnumber++) {
    pinMode(switchpins[pinnumber], INPUT_PULLUP);
    debouncedButtons[pinnumber].attach(switchpins[pinnumber]);
    debouncedButtons[pinnumber].interval(80);
    debouncedButtons[pinnumber].update();
  }
}

void turntableArmtrayperipheralsSetup() {
  pinMode(PIN_LIFTER, OUTPUT);
  pinMode(PIN_MUTING, OUTPUT);
  setLifter(armUp);
  setMute(mutesound);
}

void turntableDcmSetup() {
  for (int pinnumber = DCM1; pinnumber < MAXDCM; pinnumber++) {
    pinMode(dcmpins[pinnumber], OUTPUT);
    digitalWrite(dcmpins[pinnumber], DcmInactive);
  }
}

void turntableDdSetup() {
  pinMode(PIN_DDSS, OUTPUT);
  pinMode(PIN_DD33, OUTPUT);
  pinMode(PIN_DD45, OUTPUT);
  digitalWrite(PIN_DDSS, DdInactive);
  digitalWrite(PIN_DD33, DdInactive);
  digitalWrite(PIN_DD45, DdInactive);
}

void turntableSensorSetup() {
  pinMode(PIN_IR30, INPUT);
  pinMode(PIN_IR17, INPUT);
}

void turntableLedSetup() {
  for (int pinnumber = LED1; pinnumber < MAXLED; pinnumber++) {
    gpio_reset_pin((gpio_num_t)ledpins[pinnumber]);
    pinMode(ledpins[pinnumber], OUTPUT);
    digitalWrite(ledpins[pinnumber], LOW);
  }
}

void turntableCompuselectorSetup() {
  pinMode(PIN_COMPUSELECTOR, OUTPUT);
  digitalWrite(PIN_COMPUSELECTOR, bStop);
}

void turntablePeripheralUpdate() {
  //17 & 30cm sensors
  if (sense30()) DetectionTime[DISC30] = millis();
  if (sense17()) DetectionTime[DISC17] = millis();
  if (isArmDown()) armdowntime = millis(); //unmute timer.
  if (isTurning() && isState(DETECT)) computeAutoSpeed();
  //process Mute
  setMute(armdowntime > millis() - MUTEPOSTDOWN);
  compuselect();
  //LEDS -- P-L55
  updateLeds();
}

//lifter
int armLifter() {
  return digitalRead(PIN_LIFTER);
}

void setLifter(int lifterPosition) {
  digitalWrite(PIN_LIFTER, lifterPosition);
}

void setMute(bool activateMute) {
  digitalWrite(PIN_MUTING, activateMute ? mutesound : playsound );
}

int armPosition() {
    int counterValue = 0;
    pcnt_unit_get_count(counterUnit, &counterValue);
    return counterValue;
}

//Sensors
bool sense30() {
  return digitalRead(PIN_IR30) == irVisible;
}

bool sense17() {
  return digitalRead(PIN_IR17) == irVisible;
}

void compuselect() {
  digitalWrite(PIN_COMPUSELECTOR, isTurning()? bStop : bStart);
}

//THIS METHOD IS NOT REALLY TESTED
//LEDs are not present on P-L45 but schematic of P-L55 show the signals pass to transistor.
//The daughterboard adds a MOSFET level shifter to trigger these transistor.
void LD(int ledNumber, bool illuminate) {
  if (ledNumber >= MAXLED) return;
  if (ledNumber < LED1) return;
  digitalWrite(ledpins[ledNumber], illuminate? HIGH : LOW);
}

//============================ state machine logic ============================

bool isHome() {
  return armPosition() == Steps[HOME];
}

bool isOverPlatter() {
  return armPosition() >= Steps[START30];
}

bool isAtEndPosition() {
  return armPosition() >= Steps[END];
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
  return armLifter() == armUp;
}

bool isArmDown() {
  return armLifter() == armDown;
}

void raiseArm() {
  setLifter(armUp);
}

void lowerArm() {
  if (isOverPlatter()) setLifter(armDown);
}

void toggleArm() {
  if (armLifter() == armUp) lowerArm();
  else raiseArm();
}

void resetArmposition() {
  pcnt_unit_clear_count(counterUnit);
  initializationCompleted = true;
}

void computeAutoSpeed() {
  bool recent30cmSensed = (DetectionTime[DISC30] > millis() - DETECTIONDURATION);
  bool recent17cmSensed = (DetectionTime[DISC17] > millis() - DETECTIONDURATION);
  if (recent30cmSensed && recent17cmSensed) DiscSize = NODISC;
  else if (recent30cmSensed)                DiscSize = DISC17;
  else                                      DiscSize = DISC30;

  if (highVerbosity && previousDiscSize != DiscSize)  webSerialPrintln(String(millis()) + " - Detected " + sizename[DiscSize]);
  previousDiscSize = DiscSize;
  setAutoDDspeed();
}

bool discPresent() {
  return DiscSize != NODISC;
}

void setAutoDDspeed() {
  digitalWrite(PIN_DD45, ((DiscSize == DISC17) ^ softSpeedInverter) ? DdActive : DdInactive); //if nodisc - 33rpm
  digitalWrite(PIN_DD33, ((DiscSize == DISC30 || DiscSize == NODISC) ^ softSpeedInverter) ? DdActive : DdInactive);
}

void setDCM(int DCMNumber) {
  pcnt_channel_set_edge_action(counterChan, DCMNumber == 3 ? PCNT_CHANNEL_EDGE_ACTION_DECREASE : PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);
  for (int pinnumber = DCM1; pinnumber < MAXDCM; pinnumber++) {
    digitalWrite(dcmpins[pinnumber], DCMNumber == pinnumber ? DcmActive : DcmInactive );
  }
}

int getDCM() {
  for (int pinnumber = DCM1; pinnumber < MAXDCM; pinnumber++) {
    if (digitalRead(dcmpins[pinnumber]) == DcmActive) return pinnumber;
  }
  return 0;
}

bool DCM(int DCMNumber) {
  if (DCMNumber >= MAXDCM) return false;
  if (DCMNumber < DCM1) return false;
  return digitalRead(dcmpins[DCMNumber]) == DcmActive;
}

void moveArmOut() {
  raiseArm();
  setDCM(3);
}

void moveArmIn() {
  raiseArm();
  setDCM(1);
}

void enableServo() {
  setDCM(2);
}

void dontMove()
{
  setDCM(0);
}


void moveArmTo(uint16_t  position) {
  if (armPosition() > position) {
  	moveArmOut();
  }
  else if (armPosition() < position) {
    moveArmIn();
  }
  else {
    dontMove();
  }
}

bool armResetNotActive() {
  return debouncedButtons[ARM].read() == released;
}

bool reachedArmReset() {
  return debouncedButtons[ARM].read() ==  pressed;
}

bool reachedHome() {
  return reachedArmReset();
}

bool reachedPosition() {
  return armPosition() == desiredPosition;
}

bool reachedEndPosition() {
  return armPosition() >= Steps[END];
}

void startDD() {
  digitalWrite(PIN_DDSS, DdActive);
}

void stopDD() {
  digitalWrite(PIN_DDSS, DdInactive);
}

bool isTurning() {
  return digitalRead(PIN_DDSS) == DdActive;
}

bool dd33active() {
  return digitalRead(PIN_DD33) == DdActive;
}

bool dd45active() {
  return digitalRead(PIN_DD45) == DdActive;
}

void playRecord() {
  lowerArm();
  enableServo();
}

//Tonearm control=============================
void turntableSetup() {
  webSerialPrintln("Starting initialization sequence");

  turntableLedSetup();
  turntableCompuselectorSetup();

  turntableDcmSetup();
  changeState(INITIAL);
  turntableDdSetup();
  setAutoDDspeed();

  turntableArmtrayperipheralsSetup();
  turntableSensorSetup();
  turntableSwitchSetup();

  turntableCounterSetup();
  
  webSerialPrintln("Peripheral configuration completed");
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
  webSerialPrintln(String("armPosition:     ") + armPosition() + " (" + desiredPosition + ")");
  webSerialPrintln(String("Initialization:  ") + (initializationCompleted ? "Completed" : "Pending"));
  webSerialPrintln(String("arm lifter:      ") + (armLifter() == armUp ? "armUp" : "armDown"));
  webSerialPrintln(String("armReset switch: ") + (reachedArmReset() ? "Pressed" : "Released"));

  webSerialPrint("DCM    :");
  webSerialPrint  ((String("   1-")) + (DCM(1) ? "ON" : "--"));
  webSerialPrint  ((String("   2-")) + (DCM(2) ? "ON" : "--"));
  webSerialPrintln((String("   3-")) + (DCM(3) ? "ON" : "--"));

  webSerialPrint("DD     :");
  webSerialPrint  ((String("  SS-")) + (isTurning() ? "EN" : "--"));
  webSerialPrint  ((String("  33-")) + (dd33active() ? "EN" : "--"));
  webSerialPrintln((String("  45-")) + (dd45active() ? "EN" : "--"));

  webSerialPrint("Sensors:");
  webSerialPrint  ((String("  30-")) + (sense30() ? "IR(" : "no(") + DetectionTime[DISC30] + ")");
  webSerialPrintln((String("  17-")) + (sense17() ? "IR(" : "no(") + DetectionTime[DISC17] + ")");

  webSerialPrintln((String(sizename[DiscSize])) + (softSpeedInverter ? " | -INVERT-" : " | noinvert") + (repeat ? " | -REPEAT-" : " | norepeat"));
  webSerialPrintln("=======================================");
}

//for UI
String turntableStatus() {
 return TurntableStateDesc[currentState];
}

void updateLeds() {
  LD(1, !(isState(IDLE) || isState(PLAY))); //Compute - all but play&idle is "I am doing something"
  LD(2, repeat); //Repeat
  LD(3, DiscSize == DISC17); //17cm
  LD(4, DiscSize == DISC30); //30cm
}

void updateKeys() {
  for (int pinnumber = ARM; pinnumber < MAXSWITCH; pinnumber++) {
    debouncedButtons[pinnumber].update();
  }
}

void computeKeys() {
  if (debouncedButtons[ARM].fell()) webSerialPrintln(String(millis()) + " - Resetting armPosition");
  if (debouncedButtons[ARM].read() == pressed) resetArmposition();

  if (debouncedButtons[SWITCH1].fell()) requestRepeat();

  if (highVerbosity && debouncedButtons[SWITCH2].fell()) webSerialPrintln(String(millis()) + " - request Move IN");
  if (debouncedButtons[SWITCH2].read() == pressed) {
    moveArmIn();// stepTonearmIn(); //DEBUG REPLACE THE STEP WHICH ACTUALLY HAS VALIDATION ***TODO
  }

  if (highVerbosity && debouncedButtons[SWITCH3].fell()) webSerialPrintln(String(millis()) + " - request Move OUT");
  if (debouncedButtons[SWITCH3].read() == pressed) {
    moveArmOut();// stepTonearmOut(); //DEBUG REPLACE THE STEP WHICH ACTUALLY HAS VALIDATION ***TODO
  }

  if (debouncedButtons[SWITCH4].fell()) requestUpDown();
  if (debouncedButtons[SWITCH5].fell()) requestStartStop();
}

void requestStartStop() {
  if (highVerbosity) webSerialPrintln(String(millis()) + " - request Play/Stop");
  if (!initializationCompleted) return;

  if (highVerbosity)
  {
    webSerialPrintln("| Lifter |ArmReset |  DDSS  |"); 
    webSerialPrintln("| (p31)  | (p25)   | (p38)  |"); 
    webSerialPrint  (armLifter() == armDown ? "| DN (H) " : "| UP (L) ");
    webSerialPrint  (reachedArmReset() ? "| HOME(L) " : "| away(H) ");
    webSerialPrintln(isTurning() ? "| ON (L) |" : "| OFF(H) |");
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
       if (armLifter() == armUp   && reachedArmReset()  && !isTurning()) startAutoOperation();
  else if (armLifter() == armUp   && armResetNotActive() && !isTurning()) startDD();
  else if (armLifter() == armDown && armResetNotActive() && !isTurning()) startDD();
  else if (armLifter() == armUp   && armResetNotActive() && isTurning()) returnAndClear();
  else if (armLifter() == armDown && armResetNotActive() && isTurning()) returnAndClear();
  else if (isState(DETECT)) returnAndClear();
  else if (isState(GOHOME)) clearRepeat();
  else if (highVerbosity) webSerialPrintln(String(millis()) + " - nogood, undetermined state");
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

void requestGo17() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Go17");
  if (!initializationCompleted) return;
  nextState = IDLE;
  requestMove(Steps[START17]);
}

void requestGoStill() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Still");
  if (!initializationCompleted) return;
  nextState = IDLE;
  requestMove(armPosition());
}

void requestUpDown() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request UP/DOWN");
  if (!initializationCompleted) return;
       if (armLifter() == armUp && !isHome()) changeState(PLAY);
  else if (armLifter() == armDown && !isHome())  changeState(IDLE);
}

void requestMoveIn(uint16_t  delta) {
  if (armPosition() > Steps[END] - delta) return; //no rollover
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Move IN");
  if( !isAtEndPosition() && armLifter() == armUp) requestMove(armPosition() + delta);
}

void requestMoveOut(uint16_t  delta) {
  if (armPosition() < delta) return; //no rollover
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Move OUT");
  if( !isHome() && armLifter() == armUp) requestMove(armPosition() - delta);
}

void stepTonearmOut() {
  if( !isHome() && armLifter() == armUp) moveArmOut();
}

void stepTonearmIn() {
  if( !isHome() && armLifter() == armUp) moveArmIn();
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

void changeEspuiPanelColor(uint16_t id, ControlColor newColor) {
    Control* panel = ESPUI.getControl(id);
    if (panel != nullptr) {
      panel->color = newColor;
      ESPUI.updateControl(id);
    }
}

void changeEspuiLabelColor(uint16_t id, const char* colorHex) {
    String circleStyle = "background-color: " + String(colorHex) + espuiStatusStyle;

    ESPUI.setElementStyle(id, circleStyle.c_str());
    ESPUI.updateControl(id);
}

void changeEspuiIndicatorColor(uint16_t id, const char* colorHex) {
    String circleStyle = "background-color: " + String(colorHex) + espuiIndicatorElementStyle;

    ESPUI.setElementStyle(id, circleStyle.c_str());
    ESPUI.updateControl(id);
}

void turntableUiUpdate() {
// conditionnal ui update separated to minimize ESPUI starvation on multiple instances
  if (millis() - lastUpdateCycle1 >= 802) {
    lastUpdateCycle1 = millis();
    changeEspuiLabelColor(armStatusLabelId, statusHexColor[currentState]);
    changeEspuiIndicatorColor(repeatId, onOffIndicatorColor[repeat ? 1 : 0]); //include CSS
    ESPUI.print(dcmStatusId, dcmIcons[getDCM()]);
  }
  else if (millis() - lastUpdateCycle2 >= 872) {
    lastUpdateCycle2 = millis();
    updateWebSerial(); //serial has 15 lines of text
  }
  else if (millis() - lastUpdateCycle3 >= 955) {
    lastUpdateCycle3 = millis();
    ESPUI.updateControlValue(recordsizeLabelId, dd33active() ? "33" : "45");    
    ESPUI.setElementStyle(recordsizeLabelId, recordStyles[DiscSize]);  //include CSS
    ESPUI.print(lifterStatusId, (armLifter() == armUp) ? armIcons[1] : armIcons[0]);
  }
  else if (millis() - lastUpdateCycle4 >= 1024) {
    lastUpdateCycle4 = millis();
    ESPUI.print(armStatusLabelId, turntableStatus());
    ESPUI.print(armPositionLabelId, String(armPosition()));
    changeEspuiIndicatorColor(ledId, statusHexColor[currentState]); //include CSS
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
      moveArmOut();
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
      if (millis() - sensortimer >= 7500) {
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
      else if (armPosition() >= Steps[END]) {
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
  //handle inputs
  updateKeys();
  computeKeys();
  //handle sensors
  turntablePeripheralUpdate();

  //update GUI
  turntableUiUpdate();
}
