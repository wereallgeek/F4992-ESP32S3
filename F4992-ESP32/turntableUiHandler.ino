//================================================
// Turntable UI Handler
//
// Handles UI interaction 
//          between Controller 
//          and various communications
//
//================================================
Preferences ttConfig;
//repeats
//this needs to match ArmPositions
const uint16_t PresetDefaults[] =   {0,      188,       1050,      1444};
uint16_t ArmPresets[] =             {0,      188,       1050,      1444};

const char* onOffIndicatorColor[] = {"#2c3e50", "#e67e22"};
const char* armIcons[] =            {"\xE2\xA4\x93", "\xE2\xA8\xAA"}; 
const char* dcmIcons[] =            {"\xF0\x9F\x92\xA4", "\xE2\x87\x90", "\xE2\x8E\x8C", "\xE2\x87\x92"};

const char* statusHexColor[] =      {"#2c3e50", "#515453",      "#c9845e",   "#b95522",   "#b95522",   "#c97e22", "#497a98",   "#3489c9"};
const char* TurntableStateDesc[] =  {"Idle",    "Initializing", "Returning", "Leaving",   "Raising",   "Moving",  "Detecting", "Playing"};
const char* sizename[] =            {"NODISC", "30cm", "17cm"};

int printedDiscSize =               1;


// Variable to share between cores 0 (comms) and 1 (controller)
volatile bool armstateDirty    = false;
volatile bool repeatDirty      = false;
volatile bool dcmDirty         = false;
volatile bool webserialDirty   = false;
volatile bool dd33Dirty        = false;
volatile bool disksizeDirty    = false;
volatile bool armlifterDirty   = false;
volatile bool ttstateDirty     = false;
volatile bool armpositionDirty = false;
volatile bool ledstateDirty    = false;
volatile bool uiDd3Active      = false;
volatile int  uiArmPosition    = 0;
volatile const char* uiRecordStyle;
volatile const char* uiLifterIcon;
volatile const char* uidcmIcon;
volatile const char* uiTurntableStatus;
volatile const char* uiStatusHexColor;
volatile const char* uiOnOffIndicatorColor;

volatile bool    previousRepeat    = false;
volatile bool    previousDD33      = false;
volatile bool    previousArmLifter = true;
volatile int32_t previousPosition  = -1;
volatile int     previousDcm       = 0;
volatile int     previousDiscSize  = 0;
volatile int     previousArmState  = 0;
volatile int     previousLedState  = 0;
volatile int     previousTtState   = 0;

//Storage variables
#define DEFIRCYCLEDURATION         2290
#define DEFDETECTIONDURATION       2290
#define DEFMUTEDURATION            1000
#define DEFUPDURATION              350
#define DEFIRTRESHOLD              1500
volatile int irCycleDuration =     DEFIRCYCLEDURATION;
volatile int detectionDuration =   DEFDETECTIONDURATION;
volatile int muteDuration =        DEFMUTEDURATION;
volatile int upDuration =          DEFUPDURATION;
volatile int irTreshold =          DEFIRTRESHOLD;



// ============ Turntable user interface Change Computation =================
bool computeRepeatDirty() {
  bool repeat = getRepeatState();
  if (previousRepeat != repeat) {
    uiOnOffIndicatorColor = onOffIndicatorColor[repeat ? 1 : 0];
    previousRepeat = repeat;
    return true;
  }
  return false;
}

bool computeDcmDirty() {
  bool haschanged = false;
  int currentDcm = getDCM();
  if (previousDcm != currentDcm) haschanged = true;
  previousDcm = currentDcm;
  uidcmIcon = dcmIcons[currentDcm];
  return haschanged;
}

bool computeArmstateDirty() {
  bool haschanged = false;
  int currentState = getCurrentState();
  if (previousArmState != currentState) haschanged = true;
  previousArmState = currentState;
  uiStatusHexColor = statusHexColor[currentState];
  return haschanged;
}

bool computeTtstateDirty() {
  bool haschanged = false;
  int currentState = getCurrentState();
  if (previousTtState != currentState) haschanged = true;
  previousTtState = currentState;
  uiTurntableStatus = TurntableStateDesc[currentState];
  return haschanged;
}

bool computeLedstateDirty() {
  bool haschanged = false;
  int currentState = getCurrentState();
  if (previousLedState != currentState) haschanged = true;
  previousLedState = currentState;
  uiStatusHexColor = statusHexColor[currentState];
  return haschanged;
}

bool computeDd33Dirty() {
  bool haschanged = false;
  if (previousDD33 != dd33active()) haschanged=true;
  previousDD33 = dd33active();
  uiDd3Active = dd33active();
  return haschanged;
}

bool computeDisksizeDirty() {
  bool haschanged = false;
  int currentDiscSize = getDiscSize();
  if (printedDiscSize != currentDiscSize) haschanged = true;
  printedDiscSize = currentDiscSize;
  uiRecordStyle = recordStyles[currentDiscSize];
  return haschanged;
}

bool computeArmlifterDirty() {
  bool haschanged = false;
  bool currentArmLifter = armLifter();
  if (previousArmLifter != currentArmLifter) haschanged = true;
  previousArmLifter = currentArmLifter;
  uiLifterIcon = (currentArmLifter == getArmUpLevel()) ? armIcons[1] : armIcons[0];
  return haschanged;
}

bool computeArmpositionDirty() {
  bool haschanged = false;
  int32_t currentArmPosition = armPosition();
  if (previousPosition != currentArmPosition) haschanged = true;
  previousPosition = currentArmPosition;
  uiArmPosition = currentArmPosition;
  return haschanged;
}

void dirtyComputation() {
  if (computeArmstateDirty()) armstateDirty = true;
  if (computeRepeatDirty()) repeatDirty = true;
  if (computeDcmDirty()) dcmDirty = true;
  if (computeWebserialDirty()) webserialDirty = true;
  if (computeDd33Dirty()) dd33Dirty = true;
  if (computeDisksizeDirty()) disksizeDirty = true;
  if (computeArmlifterDirty()) armlifterDirty = true;
  if (computeTtstateDirty()) ttstateDirty = true;
  if (computeArmpositionDirty()) armpositionDirty = true;
  if (computeLedstateDirty()) ledstateDirty = true;
}

void requestComputation() {
  requestInvert(uiInvert);
  if (uiPressRepeat) requestRepeat();
  uiPressRepeat = false;
  if (uiPressMoveIn) {
    if (armPosition() >= getArmPresetValue(END)) uiPressMoveIn = false;
    else stepTonearmIn();
  }
  if (uiPressMoveOut) {
    if (armPosition() <= getArmPresetValue(HOME)) uiPressMoveOut = false;
    else stepTonearmOut();
  }
  if (uiPressUpDown) requestUpDown();
  uiPressUpDown = false;
  if (uiPressStartStop) requestStartStop(uiTypeStartStop);
  uiPressStartStop = false;

  if (uiAskMoveHome) requestHome();
  uiAskMoveHome = false;
  if (uiAskMoveEnd) requestGoEnd();
  uiAskMoveEnd = false;
  if (uiAskMove30) requestGo30();
  uiAskMove30 = false;
  if (uiAskMove17) requestGo17();
  uiAskMove17 = false;
  if (uiAskMoveNot) requestGoStill();
  uiAskMoveNot = false;

  if (uiRequestInit) requestInitBypass();
  uiRequestInit = false;

  if (uiAskReport) turntableReport();
  uiAskReport = false;
  if (uiAskInfra) turntableIrReport();
  uiAskInfra = false;
}

void turntableUiUpdate() {
// conditionnal ui update separated to minimize ESPUI starvation on multiple instances
  if (millis() - lastUpdateCycle1 >= 802) {
    lastUpdateCycle1 = millis();
    if(armstateDirty) changeEspuiLabelColor(armStatusLabelId, (char *)uiStatusHexColor);
    armstateDirty = false; 
    if(repeatDirty) changeEspuiIndicatorColor(repeatId, (char *)uiOnOffIndicatorColor);
    repeatDirty = false; 
    if(dcmDirty) ESPUI.print(dcmStatusId, (char *)uidcmIcon);
    dcmDirty = false;
  }
  else if (millis() - lastUpdateCycle2 >= 872) {
    lastUpdateCycle2 = millis();
    if(webserialDirty) updateWebSerial();
    webserialDirty = false;
  }
  else if (millis() - lastUpdateCycle3 >= 955) {
    lastUpdateCycle3 = millis();
    if (dd33Dirty) ESPUI.updateControlValue(recordsizeLabelId, uiDd3Active ? "33" : "45");
    dd33Dirty = false;
    if (disksizeDirty) ESPUI.setElementStyle(recordsizeLabelId, (char *)uiRecordStyle);
    disksizeDirty = false;
    if (armlifterDirty) ESPUI.print(lifterStatusId, (char *)uiLifterIcon);
    armlifterDirty = false;
  }
  else if (millis() - lastUpdateCycle4 >= 1024) {
    lastUpdateCycle4 = millis();
    if (ttstateDirty) ESPUI.print(armStatusLabelId, (char *)uiTurntableStatus);
    ttstateDirty = false;
    if (armpositionDirty) ESPUI.print(armPositionLabelId, String((int)uiArmPosition));
    armpositionDirty = false;
    if (ledstateDirty) changeEspuiIndicatorColor(ledId, (char *)uiStatusHexColor);
    ledstateDirty = false;
  }
}
// ============ Turntable user interface Change Computation =================


// ============ Debug console helper =================
void verboseDiskChange() {
  int currentDiscSize = getDiscSize();
  if (highVerbosity && previousDiscSize != currentDiscSize)  webSerialPrintln(String(millis()) + " - Detected " + sizename[currentDiscSize]);
  previousDiscSize = currentDiscSize;
}

void turntableReport() {
  webSerialPrintln(String("status:          ") + turntableStatus(getCurrentState()));
  webSerialPrintln(String("Next state:      ") + turntableStatus(getNextState()));
  webSerialPrintln(String("armPosition:     ") + armPosition() + " (" + getDesiredPosition() + ")");
  webSerialPrintln(String("Initialization:  ") + (getInitializationCompleted() ? "Completed" : "Pending"));
  webSerialPrintln(String("arm & mute:      ") + (armLifter() == getArmUpLevel() ? " armUp " : "armDown") + " (" + (getMute() ? "M)" : "P)"));
  webSerialPrintln(String("armReset switch: ") + (reachedArmReset() ? "Pressed" : "Released"));

  webSerialPrint("DCM    :");
  webSerialPrint  ((String("   1-")) + (DCM(1) ? "ON" : "--"));
  webSerialPrint  ((String("   2-")) + (DCM(2) ? "ON" : "--"));
  webSerialPrintln((String("   3-")) + (DCM(3) ? "ON" : "--"));

  webSerialPrint("DD     :");
  webSerialPrint  ((String("  SS-")) + (isTurning() ? "EN" : "--"));
  webSerialPrint  ((String("  33-")) + (dd33active() ? "EN" : "--"));
  webSerialPrintln((String("  45-")) + (dd45active() ? "EN" : "--"));

  turntableSensorReport();

  webSerialPrintln((String(sizename[getDiscSize()])) + (softSpeedInverter ? " | -INVERT-" : " | noinvert") + (getRepeatState() ? " | -REPEAT-" : " | norepeat"));
}

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
// ============ Debug console helper =================

// ======== LED INTERFACE =========
void ledAnimationSetState(int state, uint16_t currentPosition, uint16_t targetPosition) {
  setLedAnimationMode(state, statusHexColor[state], currentPosition, targetPosition);
}
// ======== LED INTERFACE =========

//======================== UI exposition of constants =======================
void readDetectionDurationFromStorage() {
  setDetectionDuration(ttConfig.getUShort("detectDuration", DEFDETECTIONDURATION));
}

void setDetectionDuration(int duration) {
  detectionDuration = duration;
}

int getDetectionDuration(){
  return detectionDuration;
}

void readMuteDurationFromStorage(){
  setMuteDuration(ttConfig.getUShort("muteDuration", DEFMUTEDURATION));
}

void setMuteDuration(int duration) {
  muteDuration = duration;
}

int getMuteDuration() {
  return muteDuration;
}

void readUpDurationFromStorage(){
  setUpDuration(ttConfig.getUShort("upDuration", DEFUPDURATION));
}

void setUpDuration(int duration) {
  upDuration = duration;
}

int getUpDuration() {
  return upDuration;
}

void readIrCycleDurationFromStorage() {
  setIrCycleDuration(ttConfig.getUShort("irCycleDuration", DEFIRCYCLEDURATION));
}

void setIrCycleDuration(int duration) {
  irCycleDuration = duration;
}

int getIrCycleDuration() {
  return irCycleDuration;
}

void readIrTresholdFromStorage() {
  setIrTreshold(ttConfig.getUShort("irTreshold", DEFIRTRESHOLD));
}

void setIrTreshold(int treshold) {
  irTreshold = treshold;
}

int getIrTreshold() {
  return irTreshold;
}

void ttConfigClear() {
  ttConfig.clear();
}

void readArmPresetValuesFromStorage() {
  setArmPresetValues (PresetDefaults[HOME], 
                      ttConfig.getUShort("Steps30", PresetDefaults[START30]),
                      ttConfig.getUShort("Steps17", PresetDefaults[START17]),
                      ttConfig.getUShort("StepsEnd", PresetDefaults[END]));
}

void setArmPresetValues(uint16_t valueForHome, uint16_t valueFor30, uint16_t valueFor17, uint16_t valueForEnd) {
  ArmPresets[HOME] = valueForHome;
  ArmPresets[START30] = valueFor30;
  ArmPresets[START17] = valueFor17;
  ArmPresets[END] = valueForEnd;
}

uint16_t getArmMaxValue() {
  return ArmPresets[END];
}

uint16_t getArmPresetValue(int presetIndex) {
  if (presetIndex < HOME || 
      presetIndex > END) return 0;
  return ArmPresets[presetIndex];
}

const char* turntableStatus(int stateIndex) {
  if (stateIndex < IDLE || 
      stateIndex > PLAY) return "Error";
  return TurntableStateDesc[stateIndex];
}

void ttConfigSetup() {
  ttConfig.begin("F4992");
}

void readTurntablePresetValuesFromStorage() {
  readArmPresetValuesFromStorage();
  readDetectionDurationFromStorage();
  readMuteDurationFromStorage();
  readUpDurationFromStorage();
  readIrCycleDurationFromStorage();
  readIrTresholdFromStorage();
}
//======================== UI exposition of constants =======================

//======================== TT CONFIGURATION =================================
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
//======================== TT CONFIGURATION =================================