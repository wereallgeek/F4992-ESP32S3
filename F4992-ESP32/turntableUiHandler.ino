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
const uint16_t PresetDefaults[] =   {0,      191,       1048,      1435};
uint16_t ArmPresets[] =             {0,      191,       1048,      1435};

const char* onOffIndicatorColor[] = {"#2c3e50", "#e67e22"};
const char* armIcons[] =            {"\xE2\xA4\x93", "\xE2\xA8\xAA"}; 
const char* dcmIcons[] =            {"\xF0\x9F\x92\xA4", "\xE2\x87\x90", "\xE2\x8E\x8C", "\xE2\x87\x92"};

const char* statusHexColor[] =      {"#2c3e50", "#515453",      "#c9845e",   "#b95522",   "#b95522",   "#c97e22",   "#497a98",   "#ad221d",   "#3489c9"};
const char* TurntableStateDesc[] =  {"Idle",    "Initializing", "Returning", "Leaving",   "Raising",   "Moving",    "Detecting", "Rejecting", "Playing"};
const char* TurntableSqttMedia[] =  {"idle",    "off",          "buffering", "buffering", "buffering",  "buffering", "buffering","buffering", "playing"};

const char* sizename[] =            {"NODISC", "30cm", "17cm"};

const char* record33style = "width:80px;height:80px;border-radius:50%;background:radial-gradient(circle,#000 7%,#0000 8%),radial-gradient(#555 18%,#111 19%);border:4px double #222;color:#fffc;line-height:80px;text-align:center;font-weight:700;display:inline-block;vertical-align:middle;font-size:3.2rem;margin-left:20px!important;";
const char* record45style = "width:70px;height:70px;border-radius:50%;background:radial-gradient(circle,#000 25%,#0000 26%),radial-gradient(#555 45%,#111 46%);border:4px solid #222;color:#fffc;line-height:80px;text-align:center;font-weight:700;display:inline-block;vertical-align:middle;font-size:3rem;margin-left:20px!important;";
const char* recordNodiscStyle = "width:80px;height:80px;border-radius:50%;background:radial-gradient(circle,#000 4%,#0000 5%),radial-gradient(#777 5%,#333 6%);border:2px solid #444;color:#0000;line-height:80px;text-align:center;display:inline-block;vertical-align:middle;margin-left:20px!important;";

const char* recordStyles[] =      { recordNodiscStyle, record33style, record45style };

//CSS as an add-on to color
const char* espuiStatusStyle = " !important;width:150px;height:32px;border-radius:4px;display:inline-block;border:1px solid #444;font:700 .85rem/32px sans-serif;text-align:center;vertical-align:middle;margin-top:20px!important;";
const char* espuiIndElemStyle = " !important;width:15px;height:15px;border-radius:50%;display:inline-block;vertical-align:middle;margin:5px;border:1px solid #222!important;";

//switch style
const char* swStyleOFF = "width:40px;height:22px;background:#000;border:1px solid #444;border-radius:11px;display:inline-block;vertical-align:top;transform:scale(.8);margin:-3px -5px 0!important;";
const char* swStyleON  = "width:40px;height:22px;background:#2ECC71;border:1px solid #27AE60;border-radius:11px;display:inline-block;vertical-align:top;transform:scale(.8);margin:-3px -5px 0!important;";

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
volatile bool    previousUiInvert  = true;
volatile int32_t previousPosition  = -1;
volatile int     previousDcm       = -1;
volatile int     previousDiscSize  = -1;
volatile int     previousArmState  = -1;
volatile int     previousLedState  = -1;
volatile int     previousTtState   = -1;

unsigned long lastUpdateCycle1 = 0; 
unsigned long lastUpdateCycle2 = 0; 
unsigned long lastUpdateCycle3 = 0; 
unsigned long lastUpdateCycle4 = 0; 

unsigned long beenIdleSince           = 0;
unsigned long playTimerUpdateTime     = 0;
unsigned long playtimer               = 0;
bool          playTimerIsIncrementing = false;
int32_t       positionAtPause         = 0;
bool          playTimerWasPaused      = false;

//Storage variables
#define DEFIRCYCLEDURATION         2500
#define DEFDETECTIONDURATION       3000
#define DEFMUTEDURATION            575
#define DEFTIMEOUT                 30
#define DEFTIMEOUTENABLED          false
#define DEFUPDURATION              350
#define DEFIRTRESHOLD              1500
#define DEFFFWDREWLEN              2500
volatile int irCycleDuration =     DEFIRCYCLEDURATION;
volatile int detectionDuration =   DEFDETECTIONDURATION;
volatile int muteDuration =        DEFMUTEDURATION;
volatile int playtimeout =         DEFTIMEOUT;
volatile bool playtimeoutEnabled = DEFTIMEOUTENABLED;
volatile int upDuration =          DEFUPDURATION;
volatile int irTreshold =          DEFIRTRESHOLD;
volatile int ffwdRevLenInMs =      DEFFFWDREWLEN;



// ============ Turntable user interface Change Computation =================
bool computeRepeatDirty() {
  bool repeat = getRepeatState();
  if (previousRepeat != repeat) {
    previousRepeat = repeat;
    uiOnOffIndicatorColor = onOffIndicatorColor[repeat ? 1 : 0];
    return true;
  }
  return false;
}

bool computeDcmDirty() {
  int currentDcm = getDCM();
  if (previousDcm != currentDcm) {
    previousDcm = currentDcm;  
    uidcmIcon = dcmIcons[currentDcm];
    return true;
  }
  return false;
}

bool computeArmstateDirty() {
  int currentState = getCurrentState();
  if (previousArmState != currentState) {
    previousArmState = currentState;
    uiStatusHexColor = statusHexColor[currentState];
    return true;
  }
  return false;
}

bool computeTtstateDirty() {
  int currentState = getCurrentState();
  if (previousTtState != currentState) {
    previousTtState = currentState;  
    uiTurntableStatus = TurntableStateDesc[currentState];
    return true;
  }
  return false;
}

bool computeLedstateDirty() {
  int currentState = getCurrentState();
  if (previousLedState != currentState) {
    previousLedState = currentState;  
    uiStatusHexColor = statusHexColor[currentState];
    return true;
  }
  return false;
}

bool computeDd33Dirty() {
  if (previousDD33 != dd33active()) {
    uiDd3Active = dd33active();
    previousDD33 = dd33active();  
    return true;
  }
  return false;
}

bool computeArmlifterDirty() {
  bool currentArmLifter = armLifter();
  if (previousArmLifter != currentArmLifter) {
    previousArmLifter = currentArmLifter;
    uiLifterIcon = (currentArmLifter == getArmUpLevel()) ? armIcons[1] : armIcons[0];
    return true;
  }
  return false;
}

bool computeArmpositionDirty() {
  int32_t currentArmPosition = armPosition();
  if (previousPosition != currentArmPosition) {
    uiArmPosition = currentArmPosition;
    previousPosition = currentArmPosition;
    return true;
  }
  return false;
}

void dirtyComputation() {
  if (computeArmstateDirty()) armstateDirty = true;
  if (computeRepeatDirty()) repeatDirty = true;
  if (computeDcmDirty()) dcmDirty = true;
  if (computeWebserialDirty()) webserialDirty = true;
  if (computeDd33Dirty()) dd33Dirty = true;
  if (computeArmlifterDirty()) armlifterDirty = true;
  if (computeTtstateDirty()) ttstateDirty = true;
  if (computeArmpositionDirty()) armpositionDirty = true;
  if (computeLedstateDirty()) ledstateDirty = true;
}

void requestComputation() {
  requestInvert(uiInvert);
  if (uiPressRepeat) {
    requestRepeat();
    uiPressRepeat = false;
  }
  if (uiPressMoveIn) {
    if (armPosition() >= getArmPresetValue(END)) uiPressMoveIn = false;
    else stepTonearmIn();
  }
  if (uiPressMoveOut) {
    if (armPosition() <= getArmPresetValue(HOME)) uiPressMoveOut = false;
    else stepTonearmOut();
  }
  if (uiPressUpDown) {
    requestUpDown();
    uiPressUpDown = false;
  }
  if (uiPressStartStop) {
    requestStartStop(uiTypeStartStop);
    uiPressStartStop = false;
  }
  if (uiPressJustStart) {
    requestJustStart(uiTypeStartStop);
    uiPressJustStart = false;
  }
  if (uiPressJustStop) {
    requestJustStop(uiTypeStartStop);
    uiPressJustStop = false;
  }
  if (uiAskMoveHome) {
    requestHome();
    uiAskMoveHome = false;
  }
  if (uiAskMoveEnd) {
    requestGoEnd();
    uiAskMoveEnd = false;
  }
  if (uiAskMove30) {
    requestGo30();
    uiAskMove30 = false;
  }
  if (uiAskMove17) {
    requestGo17();
    uiAskMove17 = false;
  }
  if (uiAskMoveNot) {
    requestGoStill();
    uiAskMoveNot = false;
  }
  if (uiAskMoveTo) {
    requestGoToAndPlay(uiToPosition);
    uiAskMoveTo = false;
  }

  if (uiRequestInit) {
    requestInitBypass();
    uiRequestInit = false;
  }

  if (uiAskReport) {
    turntableReport();
    uiAskReport = false;
  }
  if (uiAskInfra) {
    turntableIrReport();
    uiAskInfra = false;
  }
}

void turntableUiUpdate() {
  unsigned long currentmillis = millis();

  if (currentmillis - lastUpdateCycle1 >= 802) {
    lastUpdateCycle1 = currentmillis;
    if(armstateDirty) {
      changeEspuiLabelColor(armStatusLabelId, (char *)uiStatusHexColor);
      armstateDirty = false; 
    }
    if(repeatDirty) {
      changeEspuiIndicatorColor(repeatId, (char *)uiOnOffIndicatorColor);
      repeatDirty = false; 
    }
    if(dcmDirty) {
      ESPUI.print(dcmStatusId, (char *)uidcmIcon);
      dcmDirty = false;
    }
    if(uiInvert != previousUiInvert) {
      if (invertbuttonVisible()) reflectSwitchPosition(spd_switch, uiInvert);
      previousUiInvert = uiInvert;
    }
  }
  else if (currentmillis - lastUpdateCycle2 >= 872) {
    lastUpdateCycle2 = currentmillis;
    if(webserialDirty) updateWebSerial();
    webserialDirty = false;
  }
  else if (currentmillis - lastUpdateCycle3 >= 955) {
    lastUpdateCycle3 = currentmillis;
    if (dd33Dirty) {
      ESPUI.updateControlValue(recordsizeLabelId, uiDd3Active ? "33" : "45");
      dd33Dirty = false;
    }
    if (disksizeDirty) { 
      ESPUI.setElementStyle(recordsizeLabelId, (char *)uiRecordStyle);
      disksizeDirty = false;
    }
    if (armlifterDirty) {
      ESPUI.print(lifterStatusId, (char *)uiLifterIcon);
      armlifterDirty = false;
    }
  }
  else if (currentmillis - lastUpdateCycle4 >= 1024) {
    lastUpdateCycle4 = currentmillis;
    if (ttstateDirty) {
      ESPUI.print(armStatusLabelId, (char *)uiTurntableStatus);
      ttstateDirty = false;
    }
    if (armpositionDirty) {
      ESPUI.print(armPositionLabelId, String("Current : ") + (int)uiArmPosition);
      armpositionDirty = false;
    }
    if (ledstateDirty) {
      changeEspuiIndicatorColor(ledId, (char *)uiStatusHexColor);
      ledstateDirty = false;
    }
    if (uiAskfwupdate) {
      setEspuiFirmwareUpdateText();
      uiAskfwupdate = false;
    }    
  }
}
// ============ Turntable user interface Change Computation =================


// ============ UI helper =================
void requestCompleteStatusRedraw() {
  armlifterDirty = true;
  dd33Dirty = true;
  dcmDirty = true;
  armstateDirty = true;
  previousDiscSize = -1;
}

void verboseDiskChange(int currentDiscSize) {
  if (previousDiscSize != currentDiscSize) {
    if (highVerbosity)  webSerialPrintln(String(millis()) + " - Detected " + sizename[currentDiscSize]);
    previousDiscSize = currentDiscSize;
    uiRecordStyle = recordStyles[currentDiscSize];
    disksizeDirty = true;
  }  
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


// ================== turntable computational methods =======================
int approximateRecordNumberOfSteps() {
  return ArmPresets[END] - (getUiRecordSize() == "33") ? ArmPresets[START30] : ArmPresets[START17];
}

float approximateRecordLenght() {
  // approximate record lenght since it varies from disc to disc
  return (getUiRecordSize() == "33") ? 1320.0 : 360.0;
}

String elaboratedTurntableForState(int stateToReport, int destPos, bool discSize30, bool spd33) {
  if (stateToReport == PLAY) return String(turntableStatus(stateToReport)) + (discSize30 ? " large record at " : " small record at ") + (spd33 ? "33 rpm" : "45 rpm");
  if (stateToReport == MOVE) return String(turntableStatus(stateToReport)) + " to " + secondsToText(positionToSeconds(destPos));
  return turntableStatus(stateToReport);
}

String elaboratedTurntableStatus() {
  return elaboratedTurntableForState(getCurrentState(), getDesiredPosition(), getDiscSize() == 1, dd33active());
}

String elaboratedTimeForState(int stateToReport, float posInSecs, float lenghtInSecs) {  
  if (stateToReport == PLAY || stateToReport == UPTOMOVE || stateToReport == MOVE) return secondsToText(posInSecs) + " / ~" + secondsToText(lenghtInSecs);
  return "";
}

String elaboratedTimeStatus() {  
  return elaboratedTimeForState(getCurrentState(), elapsedPlaytimeInSeconds(), approximateRecordLenght());
}


int computePositionStepFromPercent(float targetPercent) {
  if (startStep() == endStep()) return 0;
  float constrainedTarget = constrain(targetPercent, 0.0f, 100.0f);
  float targetStep = (constrainedTarget / 100.0f) * (endStep() - startStep()) + startStep();

  return round(targetStep);
}

int currentElapsedPercent() {
  return currentElapsedPlayinSeconds() * 100 / approximateRecordLenght();
}

float currentPositionPercentWithDecimals() {
  if (startStep() == endStep()) return 0.0f;
  float percent = ((float)(uiArmPosition - startStep()) / (float)(endStep() - startStep())) * 100.0f;
  return constrain(percent, 0.0f, 100.0f);;
}

int currentPositionPercent() {
  return (int)currentPositionPercentWithDecimals();
}

int computePositionStepFromSeconds(float targetSeconds) {
  if (startStep() == endStep()) return startStep();

  float totalSeconds = approximateRecordLenght();
  if (totalSeconds <= 0.0) return startStep();

  float ratio = targetSeconds / totalSeconds;
  ratio = constrain(ratio, 0.0, 1.0);

  return startStep() + (int)(ratio * (float)(endStep() - startStep()));
}

int startStep() {
  return (getUiRecordSize() == "33") ? ArmPresets[START30] : ArmPresets[START17];
}

int endStep() {
  return ArmPresets[END];
}

float totalTravel() {
  return (float)(endStep() - startStep());
}

unsigned long stepsToMilliseconds(int steps) {
  return (unsigned long)(stepsToSeconds(steps) * 1000.0f); 
}

float stepsToSeconds(int steps) {
  float ratio = (float)steps / totalTravel();
  return ratio * approximateRecordLenght();
}

float positionToSeconds(int position) {
  float ratio = (float)(position - startStep()) / totalTravel();
  ratio = constrain(ratio, 0.0f, 1.0f);
  return ratio * approximateRecordLenght();
}

float elapsedPlaytimeInSeconds() {
  return currentElapsedPlayinSeconds();
}

float currentPositionInSeconds() {
  return positionToSeconds(uiArmPosition);
}

String getUiSizeName() {
  return sizename[previousDiscSize];
}

String getUiRecordSize() {
  return uiDd3Active ? "33" : "45";
}

int getUipreviousDcm() {
  return previousDcm;
}

int getUiArmPosition() {
  return uiArmPosition;
}

bool isPlaying() {
  return getCurrentState() == PLAY;
}
// ================== turntable computational methods =======================

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
// ============ UI helper =================

// ======== LED INTERFACE =========
void ledAnimationSetState(int state, uint16_t currentPosition, uint16_t targetPosition) {
  setLedAnimationMode(state, statusHexColor[state], currentPosition, targetPosition);
}
// ======== LED INTERFACE =========

// ======== tt-UI link ============
String getEspuiSwitchStyle(bool onstatus) {
  return onstatus ? swStyleON : swStyleOFF;
}

String getEspuiDefaultRecord() {
  return recordStyles[1];
}

String getEspuiLabelColor(String colorHex) {
  return "background-color:" + String(colorHex) + espuiStatusStyle;
}

String getEspuiIndicatorColor(String colorHex) {
  return "background-color:" + String(colorHex) + espuiIndElemStyle;
}

void changeEspuiLabelColor(uint16_t id, const char* colorHex) {
    String circleStyle = getEspuiLabelColor(String(colorHex));

    ESPUI.setElementStyle(id, circleStyle.c_str());
    ESPUI.updateControl(id);
}

void changeEspuiIndicatorColor(uint16_t id, const char* colorHex) {
    String circleStyle = getEspuiIndicatorColor(String(colorHex));

    ESPUI.setElementStyle(id, circleStyle.c_str());
    ESPUI.updateControl(id);
}
// ======== tt-UI link ============


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

void readTimeoutDurationFromStorage(){
  setTimeoutDuration(ttConfig.getUShort("t_out", DEFTIMEOUT));
}

void setTimeoutDuration(int duration) {
  playtimeout  = duration;
}

int getTimeoutDuration() {
  return playtimeout; //seconds
}

int getTimeoutDurationInMs() {
  return playtimeout * 60000;
}

void readTimeoutEnabledFromStorage(){
  setTimeoutEnabled(ttConfig.getBool("t_out_active", DEFTIMEOUTENABLED));
}

void setTimeoutEnabled(bool enable) {
  playtimeoutEnabled = enable;
}

bool getTimeoutEnabled() {
  return playtimeoutEnabled;
}

void readFfwdRewLenghtFromStorage(){
  setFfwdRewLenght(ttConfig.getUShort("revffwdlen", DEFFFWDREWLEN));
}

void setFfwdRewLenght(int lenghtInmiliseconds) {
  ffwdRevLenInMs = lenghtInmiliseconds;
}

int getFfwdRewLenght(){
  return ffwdRevLenInMs;
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
  ArmPresets[START30] = (valueFor30 > 0) ?  valueFor30  : 1;
  ArmPresets[START17] = (valueFor17 > 0) ?  valueFor17  : 1;
  ArmPresets[END]     = (valueForEnd > 0) ? valueForEnd : 1;
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

const char* turntableMediaplayerStatus(int stateIndex) {
  if (stateIndex < IDLE || 
      stateIndex > PLAY) return "off";
  if (getCurrentState() == IDLE && armPosition() > getArmPresetValue(HOME)) return "paused";
  return TurntableSqttMedia[stateIndex];
}

const char* turntableCurrentStatus() {
  return turntableStatus(getCurrentState());
}

const char* turntableCurrentMediaplayerStatus() {
  return turntableMediaplayerStatus(getCurrentState());
}

void ttConfigSetup() {
  ttConfig.begin("F4992");
}

void readTurntablePresetValuesFromStorage() {
  readArmPresetValuesFromStorage();
  readDetectionDurationFromStorage();
  readMuteDurationFromStorage();
  readTimeoutDurationFromStorage();
  readTimeoutEnabledFromStorage();
  readFfwdRewLenghtFromStorage();
  readUpDurationFromStorage();
  readIrCycleDurationFromStorage();
  readIrTresholdFromStorage();
}
//======================== UI exposition of constants =======================

//======================== elapsedTimeCounter =============================
void increasePlayTimer(unsigned long ammount) {
  playtimer += ammount;
}

void updatePlayTimer() {
  if (playTimerIsIncrementing) {
    increasePlayTimer(millis() - playTimerUpdateTime);
    playTimerUpdateTime = millis();
  }
}

unsigned long currentElapsedPlayInMs() {
  updatePlayTimer();
  return playtimer;
}

float currentElapsedPlayinSeconds() {
  return (float)currentElapsedPlayInMs() / 1000.0f;
}

void resetElapsedTimer(bool resetTimer) {
  if (resetTimer) {
    playtimer = 0;
    playTimerWasPaused = false;
    playTimerIsIncrementing = false;
  }
}

void startElapsedTimer() {
  playTimerUpdateTime = millis();
  if (playTimerWasPaused) increasePlayTimer(stepsToMilliseconds(armPosition() - positionAtPause));
  playTimerWasPaused = false;
  playTimerIsIncrementing = true;
}

void pauseElapsedTimer() {
  updatePlayTimer();
  positionAtPause = armPosition();
  playTimerWasPaused = true;
  playTimerIsIncrementing = false;
}

void setElapsedTimer(int previousState, int nextState) {
  if (previousState == nextState) return;
  if (previousState == PLAY) pauseElapsedTimer();
  if (nextState == PLAY) startElapsedTimer();
}
//======================== elapsedTimeCounter =============================

//======================== time out auto return =============================
void setTimeoutTimer(bool goingIdle) {
  if (goingIdle) beenIdleSince = millis();
}

bool leftUnattendedForTooLong() {
  if ((!playtimeoutEnabled) || (!getCurrentState() == IDLE) || (!isTurning())) return false;
  if (millis() - beenIdleSince >= getTimeoutDurationInMs()) return true;
  return false;
}
//======================== time out auto return =============================

//======================== TT CONFIGURATION =================================
//Handling config changes
void outputTurntableDetailsValues() {
  if(highVerbosity) {
    webSerialPrintln(String("Detection phase duration ") + getDetectionDuration() + String(" ms"));
    webSerialPrintln(String("Needledrop mute duration ") + getMuteDuration() + String(" ms"));
    webSerialPrintln(String("Turntable timeout ") + getTimeoutEnabled()? "Enabled, duration is " : "disabled, duration would be " + getTimeoutDuration() + String(" min"));
    webSerialPrintln(String("FFWD & REW lenght ") + getFfwdRewLenght() + String(" ms"));
    webSerialPrintln(String("IR cycle duration ") + getIrCycleDuration() + String(" ms"));
    webSerialPrintln(String("Infrared Treshold ") + getIrTreshold());
    webSerialPrintln(String("Arm Presets [0, ") + getArmPresetValue(START30) + String(", ") + getArmPresetValue(START17) + String(", ") + getArmPresetValue(END)+ String("]"));
  }
}

void resyncTurntableDetailsToScreen() {
  ESPUI.updateControlValue(detectionDurationLabelId, String(getDetectionDuration()));
  ESPUI.updateControlValue(muteDurationLabelId, String(getMuteDuration()));
  ESPUI.updateControlValue(timeoutLabelId, String(getTimeoutDuration()));
  ESPUI.updateControlValue(timeoutEnabledLabelId, String(getTimeoutEnabled()));
  ESPUI.updateControlValue(ffwdRewSkipAmmountLabelId, String(getFfwdRewLenght()));
  ESPUI.updateControlValue(irCycleDurationLabelId, String(getIrCycleDuration()));
  ESPUI.updateControlValue(irTresholdLabelId, String(getIrTreshold()));
  ESPUI.updateControlValue(armPresetValue30LabelId, String(getArmPresetValue(START30)));
  ESPUI.updateControlValue(armPresetValue17LabelId, String(getArmPresetValue(START17)));
  ESPUI.updateControlValue(armPresetValueEndLabelId, String(getArmPresetValue(END)));
}

void applyTurntableDetailsToMemory() {
  setDetectionDuration(ESPUI.getControl(detectionDurationLabelId)->value.toInt());
  setMuteDuration(ESPUI.getControl(muteDurationLabelId)->value.toInt());
  setTimeoutDuration(ESPUI.getControl(timeoutLabelId)->value.toInt());
  setTimeoutEnabled(ESPUI.getControl(timeoutEnabledLabelId)->value.toInt());
  setFfwdRewLenght(ESPUI.getControl(ffwdRewSkipAmmountLabelId)->value.toInt());
  setIrCycleDuration(ESPUI.getControl(irCycleDurationLabelId)->value.toInt());
  setIrTreshold(ESPUI.getControl(irTresholdLabelId)->value.toInt());
  setArmPresetValues(0, ESPUI.getControl(armPresetValue30LabelId)->value.toInt(), 
                        ESPUI.getControl(armPresetValue17LabelId)->value.toInt(),
                        ESPUI.getControl(armPresetValueEndLabelId)->value.toInt());
}

void saveTurntableDetailsToConfig() {
    ttConfig.putUShort("detectDuration", (uint16_t)getDetectionDuration());
    ttConfig.putUShort("muteDuration", (uint16_t)getMuteDuration());
    ttConfig.putUShort("t_out", (uint16_t)getTimeoutDuration());
    ttConfig.putBool("t_out_active", getTimeoutEnabled());
    ttConfig.putUShort("revffwdlen", (uint16_t)getFfwdRewLenght());
    ttConfig.putUShort("irCycleDuration", (uint16_t)getIrCycleDuration());
    ttConfig.putUShort("irTreshold", (uint16_t)getIrTreshold());
    ttConfig.putUShort("Steps30", (uint16_t)getArmPresetValue(START30));
    ttConfig.putUShort("Steps17", (uint16_t)getArmPresetValue(START17));
    ttConfig.putUShort("StepsEnd", (uint16_t)getArmPresetValue(END));
}
//======================== TT CONFIGURATION =================================