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
#define PIN_COUNTER              6   

//Infrared Sensors
#define PIN_IR30                 1
#define PIN_IR17                 3

// Switches (Inputs)
#define PIN_SW1                  5
#define PIN_SW2                  7
#define PIN_SW3                  8
#define PIN_SW4                  9
#define PIN_SW5                  10
#define PIN_ARMRESET             16

// DCM Arm tray (Outputs)
#define PIN_DCM1                 17  
#define PIN_DCM2                 14
#define PIN_DCM3                 4   
#define PIN_LIFTER               13
#define PIN_MUTING               18  

// Direct Drive module
#define PIN_DDSS                 11
#define PIN_DD33                 15
#define PIN_DD45                 2

// LEDs (Outputs)
#define PIN_LED1                 39
#define PIN_LED2                 40
#define PIN_LED3                 42
#define PIN_LED4                 41

//compuselector. 
#define PIN_COMPUSELECTOR        12




enum Hardwareswitch          {ARM,          SWITCH1,  SWITCH2,    SWITCH3,    SWITCH4,   SWITCH5, MAXSWITCH};
const byte switchpins[] =    {PIN_ARMRESET, PIN_SW1,  PIN_SW2,    PIN_SW3,    PIN_SW4,    PIN_SW5};
Bounce debouncedButtons[MAXSWITCH]; 

//align hardware number with logical number and add minimum and maximum values.
enum Hardwareled             {NOLED,        LED1,         LED2,      LED3,      LED4,       MAXLED};
const byte ledpins[] =       {0,            PIN_LED1,     PIN_LED2,  PIN_LED3,  PIN_LED4};

enum Hardwaredcm             {NODCM,        DCM1,         DCM2,      DCM3,      MAXDCM};
const byte dcmpins[] =       {0,            PIN_DCM1,     PIN_DCM2,  PIN_DCM3};


const int armUp =         LOW;
const int armDown =       HIGH;

const int mutesound =     LOW;
const int playsound =     HIGH;

const int released =      HIGH;
const int pressed =       LOW;

const int DdActive =      LOW;
const int DdInactive =    HIGH;

const bool DcmActive =    HIGH;
const bool DcmInactive =  LOW;

const bool IN =           true;
const bool OUT =          false;

const int irNotvitible =  HIGH;
const int irVisible =     LOW;
const int irMinimum =     1;

const int bStop =         HIGH;
const int bStart =        LOW;

//purposely match arm position and speed
enum DetectedSize                 {NODISC, DISC30, DISC17};
const char* recordStyles[] =      { recordNodiscStyle, record33style, record45style };
DetectedSize DiscSize =           NODISC;
enum ArmPositions                 {HOME, START30, START17, END};
uint16_t  desiredPosition =       0;

enum TurntableState                {IDLE,      INITIAL,        GOHOME,      UPTOHOME,    UPTOMOVE,    MOVE,      DETECT,      PLAY};

TurntableState currentState =      IDLE;
TurntableState nextState =         IDLE;


//prototypes to make arduino IDE happy 
void changeState(TurntableState newState); 
bool isState(TurntableState state);

bool initializationCompleted = false;

unsigned long sensortimer = 0;
unsigned long DetectionTime[3] = {0, 0, 0};
unsigned long armdowntime = 0;
unsigned long armuptime = 0;

volatile bool repeat = false;

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
    .level_gpio_num = PIN_DCM3,
    .flags = { .io_loop_back = 0 },
  };
  pcnt_new_channel(counterUnit, &counterChanCfg, &counterChan);

  //action
  pcnt_channel_set_edge_action(counterChan, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);
  pcnt_channel_set_level_action(counterChan, PCNT_CHANNEL_LEVEL_ACTION_INVERSE, PCNT_CHANNEL_LEVEL_ACTION_KEEP);

  //glitch filtering
  pcnt_glitch_filter_config_t filter_config = { .max_glitch_ns = 20000 };
  pcnt_unit_set_glitch_filter(counterUnit, &filter_config);

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

//---------update-----------------
void turntablePeripheralUpdate() {
  //17 & 30cm sensors
  if (sense30()) DetectionTime[DISC30] = millis();
  if (sense17()) DetectionTime[DISC17] = millis();
  //armdown timer
  if (isArmDown()) armdowntime = millis();
  else armuptime = millis();
  //reset switch
  if (isHome()) resetDiskSize();
  //process Mute
  setMute(millis() - armuptime < getMuteDuration());
  compuselect();
  //LEDS -- P-L55
  updateLeds();
}

//--getters----------------------
int armLifter() {
  return digitalRead(PIN_LIFTER);
}

void setLifter(int lifterPosition) {
  digitalWrite(PIN_LIFTER, lifterPosition);
}

void setMute(bool activateMute) {
  digitalWrite(PIN_MUTING, activateMute ? mutesound : playsound );
}

bool getMute() {
  return digitalRead(PIN_MUTING) == mutesound;
}


int32_t armPosition() {
    int counterValue = 0;
    pcnt_unit_get_count(counterUnit, &counterValue);
    return (int32_t)counterValue;
}

//Sensors
int analogWakeAndRead(int pinNumber) {
  int analogValue = analogRead(pinNumber); //wake up!
  for (int i = 0; i < 3; i++) {
    if (analogValue != 0) break; 
    delayMicroseconds(18);
    analogValue = analogRead(pinNumber);
  }
  return analogValue;
}

int value30cm() {
  return analogWakeAndRead(PIN_IR30);
}

int value17cm() {
  return analogWakeAndRead(PIN_IR17);
}

bool sense30() {
  int sensed30value = value30cm();
  return irMinimum < sensed30value && sensed30value < getIrTreshold();
}

bool sense17() {
  int sensed17value = value17cm();
  return irMinimum < sensed17value && sensed17value < getIrTreshold();
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

bool getRepeatState() {
  return repeat;
}

bool getInitializationCompleted() {
  return initializationCompleted;
}

uint16_t getDesiredPosition() {
  return desiredPosition;
}

int getDiscSize() {
  return (int)DiscSize;
}

int getCurrentState() {
  return (int)currentState;
}

int getNextState() {
  return (int)nextState;
}

int getArmUpLevel() {
  return armUp;
}

//============================ state machine logic ============================

bool isHome() {
  return armPosition() == getArmPresetValue(HOME);
}

bool isOverPlatter() {
  return armPosition() >= getArmPresetValue(START30);
}

bool isAtEndPosition() {
  return armPosition() >= getArmPresetValue(END);
}

void changeState(TurntableState newState) {
  if(firstPassCompleted && highVerbosity) webSerialPrint(String(millis()) + " - "); //state change remains in low verbosity
  if(firstPassCompleted) webSerialPrintln(String("State: ") + turntableStatus(currentState) + " -> " + turntableStatus(newState));
  currentState = newState;
  ledAnimationSetState(currentState);
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
  if (isMovingOut()) changeState(IDLE);
}

void resetDiskSize() {
  DiscSize = NODISC;
  verboseDiskChange();
}

void computeDiscSize() {
  bool recent30cmSensed = (DetectionTime[DISC30] > millis() - getIrCycleDuration());
  bool recent17cmSensed = (DetectionTime[DISC17] > millis() - getIrCycleDuration());
  if (recent30cmSensed && recent17cmSensed) DiscSize = NODISC;
  else if (recent30cmSensed)                DiscSize = DISC17;
  else                                      DiscSize = DISC30;
  verboseDiskChange();
}

bool discPresent() {
  return DiscSize != NODISC;
}

void setAutoDDspeed() {
  digitalWrite(PIN_DD45, ((DiscSize == DISC17) ^ softSpeedInverter) ? DdActive : DdInactive); //if nodisc - 33rpm
  digitalWrite(PIN_DD33, ((DiscSize == DISC30 || DiscSize == NODISC) ^ softSpeedInverter) ? DdActive : DdInactive);
}

void setDCM(int DCMNumber) {
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

bool isMovingOut() {
  return DCM(3);
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
  return armPosition() >= getArmPresetValue(END);
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

  readTurntablePresetValuesFromStorage();
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
  changeState(UPTOHOME);
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

//Handling config changes
void outputTurntableDetailsValues() {
  if(highVerbosity) {
    webSerialPrintln(String("Detection phase duration ") + getDetectionDuration() + String(" ms"));
    webSerialPrintln(String("Needledrop mute duration ") + getMuteDuration() + String(" ms"));
    webSerialPrintln(String("IR cycle duration ") + getIrCycleDuration() + String(" ms"));
    webSerialPrintln(String("Infrared Treshold ") + getIrTreshold());
    webSerialPrintln(String("Arm Presets [0, ") + getArmPresetValue(START30) + String(", ") + getArmPresetValue(START17) + String(", ") + getArmPresetValue(END)+ String("]"));
  }
}

void resyncTurntableDetailsToScreen() {
  ESPUI.updateControlValue(detectionDurationLabelId, String(getDetectionDuration()));
  ESPUI.updateControlValue(muteDurationLabelId, String(getMuteDuration()));
  ESPUI.updateControlValue(irCycleDurationLabelId, String(getIrCycleDuration()));
  ESPUI.updateControlValue(irTresholdLabelId, String(getIrTreshold()));
  ESPUI.updateControlValue(armPresetValue30LabelId, String(getArmPresetValue(START30)));
  ESPUI.updateControlValue(armPresetValue17LabelId, String(getArmPresetValue(START17)));
  ESPUI.updateControlValue(armPresetValueEndLabelId, String(getArmPresetValue(END)));
}

void applyTurntableDetailsToMemory() {
  setDetectionDuration(ESPUI.getControl(detectionDurationLabelId)->value.toInt());
  setMuteDuration(ESPUI.getControl(muteDurationLabelId)->value.toInt());
  setIrCycleDuration(ESPUI.getControl(irCycleDurationLabelId)->value.toInt());
  setIrTreshold(ESPUI.getControl(irTresholdLabelId)->value.toInt());
  setArmPresetValues(0, ESPUI.getControl(armPresetValue30LabelId)->value.toInt(), 
                        ESPUI.getControl(armPresetValue17LabelId)->value.toInt(),
                        ESPUI.getControl(armPresetValueEndLabelId)->value.toInt());
}

void saveTurntableDetailsToConfig() {
    ttConfig.putUShort("detectDuration", (uint16_t)getDetectionDuration());
    ttConfig.putUShort("muteDuration", (uint16_t)getMuteDuration());
    ttConfig.putUShort("irCycleDuration", (uint16_t)getIrCycleDuration());
    ttConfig.putUShort("irTreshold", (uint16_t)getIrTreshold());
    ttConfig.putUShort("Steps30", (uint16_t)getArmPresetValue(START30));
    ttConfig.putUShort("Steps17", (uint16_t)getArmPresetValue(START17));
    ttConfig.putUShort("StepsEnd", (uint16_t)getArmPresetValue(END));
}

//Tonearm control=============================
void turntableSensorReport() {
  webSerialPrint("Sensors:");
  webSerialPrint  ((String("  30-")) + (sense30() ? "IR(" : "no(") + DetectionTime[DISC30] + ")");
  webSerialPrintln((String("  17-")) + (sense17() ? "IR(" : "no(") + DetectionTime[DISC17] + ")");
}

void turntableIrReport() {
  webSerialPrintln("========Infrared=sensor=report=========");
  turntableSensorReport();
  webSerialPrint("IR value:");
  webSerialPrint  ((String("  30-")) + value30cm());
  webSerialPrintln((String("  17-")) + value17cm());
  webSerialPrintln("=======================================");
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
  if (debouncedButtons[ARM].fell()) {
    if (highVerbosity) webSerialPrint(String(millis()) + " - ");
    webSerialPrintln("Resetting armPosition");
  }
  if (debouncedButtons[ARM].read() == pressed) resetArmposition();
  if (debouncedButtons[ARM].rose()) resetArmposition();

  if (debouncedButtons[SWITCH1].fell()) requestRepeat();

  if (highVerbosity && debouncedButtons[SWITCH2].fell()) webSerialPrintln(String(millis()) + " - request Move IN");
  if (debouncedButtons[SWITCH2].read() == pressed) {
    stepTonearmIn();
  }

  if (highVerbosity && debouncedButtons[SWITCH3].fell()) webSerialPrintln(String(millis()) + " - request Move OUT");
  if (debouncedButtons[SWITCH3].read() == pressed) {
    stepTonearmOut();
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
  else if (isState(GOHOME) || isState(UPTOHOME)) clearRepeat();
  else changeState(UPTOHOME); //undetermined state, going home.
}

void requestHome() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request HOME");
  nextState = IDLE;
  changeState(UPTOHOME);
}

void requestGoEnd() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request GoEND");
  if (!initializationCompleted) return;
  nextState = IDLE;
  requestMove(getArmPresetValue(END));
}

void requestGo30() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Go30");
  if (!initializationCompleted) return;
  nextState = IDLE;
  requestMove(getArmPresetValue(START30));
}

void requestGo17() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Go17");
  if (!initializationCompleted) return;
  nextState = IDLE;
  requestMove(getArmPresetValue(START17));
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
  changeState(UPTOMOVE);
}

void requestRepeat() {
  if(highVerbosity) webSerialPrintln(String(millis()) + " - request Repeat");
  repeat = !repeat;
}

void requestInvert(bool invertRequested) {
  if(highVerbosity && softSpeedInverter != invertRequested) webSerialPrintln(String(millis()) + " - request " + (invertRequested ? "[INV] speed" : "[NOR] speed"));
  softSpeedInverter = invertRequested;
}

void requestInitBypass() {
  initializationCompleted = true;
  changeState(IDLE);
}

void changeEspuiLabelColor(uint16_t id, const char* colorHex) {
    String circleStyle = "background-color: " + String(colorHex) + espuiStatusStyle;

    ESPUI.setElementStyle(id, circleStyle.c_str());
    ESPUI.updateControl(id);
}

void changeEspuiIndicatorColor(uint16_t id, const char* colorHex) {
    String circleStyle = "background-color: " + String(colorHex) + espuiIndElemStyle;

    ESPUI.setElementStyle(id, circleStyle.c_str());
    ESPUI.updateControl(id);
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
      moveArmOut();
      if (reachedArmReset()) changeState(IDLE);
      break;
    case GOHOME:
      nextState = IDLE;
      desiredPosition = getArmPresetValue(HOME);
      moveArmOut();
      if (reachedHome()) changeState(nextState);
      break;
    case UPTOHOME:
      raiseArm();
      if (armdowntime < millis() - getUpDuration()) changeState(GOHOME);
      break;
    case UPTOMOVE:
      raiseArm();
      if (armdowntime < millis() - getUpDuration()) changeState(MOVE);
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
      desiredPosition = getArmPresetValue(DiscSize);
      moveArmTo(getArmPresetValue(DISC30)); // begin arm movement to larger disc
      //section 2 automatic disk selection timing says Input for 2.5 sec
      // It is about the time it takes for the arm to get to large disc drop location.
      if (millis() - sensortimer >= (getDetectionDuration())) {
        computeDiscSize();
        desiredPosition = getArmPresetValue(DiscSize);
        moveArmTo(desiredPosition);
        if (DiscSize == NODISC)
        {
          nextState = IDLE;
          changeState(GOHOME); 
        }
        else
        {
          nextState = PLAY;
          changeState(MOVE); 
        }
      }
      //set record speed and change state
      break;
    case PLAY:
      setAutoDDspeed();
      if (isOverPlatter() && isTurning()) playRecord();
      
      if (reachedEndPosition()) {
        if (repeat) {
          nextState = PLAY;
          desiredPosition = getArmPresetValue(DiscSize);
          repeat = false; //no infinite repeat
          changeState(UPTOMOVE); 
        }
        else {
          changeState(UPTOHOME);
        }
      }
      break;
    }
  //handle inputs
  updateKeys();
  computeKeys();
  //handle sensors
  turntablePeripheralUpdate();

  //update GUI values - let GUI handle the rest
  dirtyComputation();
  requestComputation();

  //update LedPixels
  animateLeds();
}