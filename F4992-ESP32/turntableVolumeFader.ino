//================================================
// Turntable Volume Fader
//
// Handles optionnal volume fader
//
//================================================
Preferences volumeControlSetup;
volatile bool volumeFaderActive = true;
volatile int minpwmvalue = 0;

const long volumeDelay        = 21;
unsigned long volumeFaderTime = 0;

volatile int desiredVolumePercent = 0;
volatile bool muteActivation = false;
volatile int currentVolumePercent = 0;

volatile int volumePwmValue = 0;
int previouwVolumePwmValue = 0;

void setupVolumeFader() {
  volumeControlSetup.begin("volumesetup", false);
  loadVolumeControlEnable();
  loadDesiredVolumePercent();
  loadMinpwmvalue();
}

void loadVolumeControlEnable() {
  volumeFaderActive = volumeControlSetup.getBool("volctrl", false);
}

void loadDesiredVolumePercent() {
  desiredVolumePercent = volumeControlSetup.getInt("dvolpct", 100);
}

void loadMinpwmvalue() {
  minpwmvalue = volumeControlSetup.getInt("minpwm", 125);
}

void storeVolumeControlEnable(bool newEnabledState) {
  if (newEnabledState != volumeControlSetup.getBool("volctrl", false)) volumeControlSetup.putBool("volctrl", newEnabledState);
}

void storeCurrentDesiredVolumePercent() {
  if (desiredVolumePercent != volumeControlSetup.getInt("dvolpct", 100)) volumeControlSetup.putInt("dvolpct", desiredVolumePercent);
}

void storeMinpwmvalue(int newPwm) {
  if (newPwm != volumeControlSetup.getInt("minpwm", 125)) volumeControlSetup.putInt("minpwm", newPwm);
}

bool volumeChangerActivated() {
  return volumeFaderActive;
}

int currentMinPwm() {
  return minpwmvalue;
}

void setDesiredVolumePercent(int newVolume) {
  if (newVolume < 0 || newVolume > 100) return;
  desiredVolumePercent = newVolume;
}

void setDesiredVolumeZeroOne(float newVolume) {
  if (newVolume < 0 || newVolume > 1) return;
  desiredVolumePercent = (int)(newVolume * 100);
}

void activateSoundMute(bool toMute) {
  muteActivation = toMute;
  if (muteActivation) writeMuteValue(0);
}

int getVolumePercent() {
  return currentVolumePercent;
}

int getDesiredVolumePercent() {
  return desiredVolumePercent;
}

float getDesiredVolumeZeroOne() {
  float volumeZeroOne = (float)desiredVolumePercent / 100.0f;
  if (volumeZeroOne < 0.0f) return 0.0f;
  if (volumeZeroOne > 1.0f) return 1.0f;
  return volumeZeroOne;
}

int getVolumePWM() {
  return volumePwmValue;
}

bool getVolumeMuteActivation() {
  return muteActivation;
}

int computeVolumePwm() {
  if (currentVolumePercent <= 0) return 0;
  return map(currentVolumePercent, 0, 100, minpwmvalue, 255);
}

void volumeChanges() {
  int currentTarget = muteActivation ? 0 : desiredVolumePercent;
  if (currentVolumePercent == currentTarget) return;
  currentVolumePercent = currentVolumePercent + ((currentVolumePercent > currentTarget) ? -1 : 1);
  volumePwmValue = computeVolumePwm();
}

void writeMuteValue(int pwmToActivate) {
  if (!volumeFaderActive) return;
  int safepwm = pwmToActivate;
  if (pwmToActivate < 0)   safepwm = 0;
  if (pwmToActivate > 255) safepwm = 255;
  analogWrite(PIN_MUTING, safepwm);
  previouwVolumePwmValue = pwmToActivate;
}

void volumeFaderLoop() {
  if (!volumeFaderActive) return;
  if (volumeFaderTime < millis() - volumeDelay)
  {
    volumeChanges();
    volumeFaderTime = millis();
    if (previouwVolumePwmValue != volumePwmValue) {
      writeMuteValue(volumePwmValue);      
    }
  }
}