//================================================
// Turntable Volume Fader
//
// Handles optionnal volume fader
//
//================================================
const bool volumeFaderActive = true; //TODO make it tunable.
const long volumeDelayFast = 10;
const long volumeDelaySlow = 100;
unsigned long volumeFaderTime = 0;

volatile int desiredVolumePercent = 0;
volatile bool muteActivation = false;
volatile int currentVolumePercent = 0;

volatile int volumePwmValue = 0;
int previouwVolumePwmValue = 0;

void setupVolumeFader() {
  desiredVolumePercent = 100;
}

bool volumeChangerActivated() {
  return volumeFaderActive;
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
  return map(currentVolumePercent, 0, 100, 125, 255); //TODO 125 is tunable. where do we set the lower value?
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