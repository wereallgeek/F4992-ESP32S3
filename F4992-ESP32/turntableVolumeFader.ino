//================================================
// Turntable Volume Fader
//
// Handles optionnal volume fader
//
//================================================
Preferences volumeControlSetup;
volatile bool volumeFaderActive = true;
volatile int minpwmvalue = 0;

const long volumeDelayFast = 10;
const long volumeDelaySlow = 100;
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

long activeVolumeDelay() {
  return muteActivation? volumeDelayFast : volumeDelaySlow;
}

void setDesiredVolumePercent(int newVolume) {
  if (newVolume < 0 || newVolume > 100) return;
  desiredVolumePercent = newVolume;
}

void activateSoundMute(bool toMute) {
  muteActivation = toMute;
}

int getVolumePercent() {
  return currentVolumePercent;
}

int getDesiredVolumePercent() {
  return desiredVolumePercent;
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

void volumeFaderLoop() {
  if (!volumeFaderActive) return;
  if (volumeFaderTime < millis() - activeVolumeDelay())
  {
    volumeChanges();
    volumeFaderTime = millis();
    if (previouwVolumePwmValue != volumePwmValue) {
      analogWrite(PIN_MUTING, volumePwmValue);
      previouwVolumePwmValue = volumePwmValue;
    }
  }
}