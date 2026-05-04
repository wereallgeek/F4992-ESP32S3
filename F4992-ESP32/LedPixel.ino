//================================================
// LedPixel handler
//
// assist the turntable in led animation (addon)
//
//================================================
#include <Adafruit_NeoPixel.h>
// NeoPixel Extension 
#define PIN_LEDPIXEL         48  

Preferences ledsetup;
Adafruit_NeoPixel strip(1, PIN_LEDPIXEL, NEO_GRB + NEO_KHZ800);

volatile bool ledPixelEnable     = false;
volatile int  availablePixels    = 1;
volatile bool newLedPixelEnable  = false;
volatile int  newAvailablePixels = 1;

//these needs to be a 1:1 match for the turntable state machine
enum LedAnimation {NONE, WAKEPULSE, ANIMHOME, UPHOME, UPMOVE, MOVECHASER, PULSEDETECT, FOLLOWPLAY};
LedAnimation animationStyle = NONE;
enum RGB {RgbR, RgbG, RgbB};
int baseRgb[3] = {0, 0, 0};
int desiredBrightness = 50;

//chaser
uint32_t chaserColor = strip.Color(255, 0, 0);
int chaserRangeFrom = 0;
int chaserRangeTo = 5;
int chaserSpeed = 111; //for a 1sec animation 1000/number of pixels
bool chaserDirection = true;
static int currentChaserStep = -1;

bool setupStarted = false;

//=================================================================================================
void ledPixelSetup() {
  ledsetup.begin("ledsetup", false);
  loadLedPixelEnable();
  loadLedPixelNumber();

  strip.begin();
  strip.updateLength(availablePixels);
  setLedPixelBrightness(ledPixelEnable ? desiredBrightness : 0);
  strip.show();
}

void startLedPixels(bool enabled) {
  ledPixelEnable = enabled;
  strip.clear();
  strip.updateLength(availablePixels);
  setLedPixelBrightness(ledPixelEnable ? desiredBrightness : 0);
}

void activateNumberOfLeds(int nbleds) {
  availablePixels = nbleds;
  strip.clear();
  strip.updateLength(nbleds);
}

void changeLedPixelEnable(bool enable) {
  newLedPixelEnable = enable;
}

void storeLedPixelEnable() {
  ledsetup.putBool("ledstrip", newLedPixelEnable);
  startLedPixels(newLedPixelEnable);
}

void loadLedPixelEnable() {
  ledPixelEnable = ledsetup.getBool("ledstrip", false);
  newLedPixelEnable = ledPixelEnable;
}

bool useLedPixel() {
  return ledPixelEnable;
}

void changeLedPixelNumber(int nbPixels) {
  newAvailablePixels = (nbPixels < 1)? 1 : nbPixels;
}

void storeLedPixelNumber() {
  ledsetup.putInt("nbPixels", newAvailablePixels);
  activateNumberOfLeds(newAvailablePixels);
}

void loadLedPixelNumber() {
  availablePixels = ledsetup.getInt("nbPixels", 1);
  newAvailablePixels = availablePixels;
}

int numberOfPixels() {
  return availablePixels;
}

void maintainSetupValues() {
  if (newLedPixelEnable != ledPixelEnable) storeLedPixelEnable();
  if (newAvailablePixels != availablePixels) storeLedPixelNumber();
}

void setAnimationStyle(int style) {
  if (style < NONE || style > FOLLOWPLAY) animationStyle = NONE;
  else animationStyle = (LedAnimation)style;
}

void setbaseRgb(const char* hexColor) {
  int ledR, ledG, ledB;
  if (sscanf(hexColor, "#%02x%02x%02x", &ledR, &ledG, &ledB) == 3) {
    setbaseRgb(ledR, ledG, ledB);
  }
}

void setbaseRgb(int ledR, int ledG, int ledB) {
  if (ledR < 0 || ledR > 255 ||
      ledG < 0 || ledG > 255 ||
      ledB < 0 || ledB > 255 ) return;
  baseRgb[RgbR] = ledR;
  baseRgb[RgbG] = ledG;
  baseRgb[RgbB] = ledB;
}

void setDesiredBrightness(int brightness) {
  if (brightness < 0 || brightness > 255) return;
  desiredBrightness = brightness;
}

bool computeLedOverlapTransitionFromArmPosition(uint16_t stepToCompute) {
  float progress = (float)(stepToCompute * (numberOfPixels() - 1)) / getArmMaxValue();
  return (progress - (int)progress) > 0.5;
}

int computeLedFromArmPosition(uint16_t stepToCompute) {
  return (stepToCompute * (numberOfPixels() - 1)) / getArmMaxValue();
}

void setLedAnimationMode(int style, const char* hexColor, uint16_t currentPosition, uint16_t targetPosition) {
  setAnimationStyle(style);
  setbaseRgb(hexColor);
  setupChaser(computeLedFromArmPosition(currentPosition), computeLedFromArmPosition(targetPosition));
}

void setLedPixelHexColor(int ledIndex, const char* hexColor) {
  int ledR, ledG, ledB;
  if (sscanf(hexColor, "#%02x%02x%02x", &ledR, &ledG, &ledB) == 3) {
    setLedPixelRgbColor(ledIndex, ledR, ledG, ledB);
  }
}

void setLedPixelHexColor(const char* hexColor) {
  for (int ledIndex = 0; ledIndex < numberOfPixels(); ledIndex++) {
    setLedPixelHexColor(ledIndex, hexColor);
  }
}

void setLedPixelRgbColor(int ledIndex, int ledR, int ledG, int ledB) {
  if (ledR < 0 || ledR > 255 ||
      ledG < 0 || ledG > 255 ||
      ledB < 0 || ledB > 255 ||
      ledIndex < 0 || ledIndex >= numberOfPixels()) return;
  strip.setPixelColor(ledIndex, strip.Color(ledR, ledG, ledB));
}


void setLedPixelRgbColor(int ledR, int ledG, int ledB) {
  for (int ledIndex = 0; ledIndex < numberOfPixels(); ledIndex++) {
    setLedPixelRgbColor(ledIndex, ledR, ledG, ledB);
  }
}

void setLedPixelRgbColorToPreset() {
  setLedPixelRgbColor(baseRgb[RgbR], baseRgb[RgbG], baseRgb[RgbB]);
}

void setLedPixelRgbColorToPreset(int ledIndex) {
  setLedPixelRgbColor(ledIndex, baseRgb[RgbR], baseRgb[RgbG], baseRgb[RgbB]);
}

void setLedPixelBrightness(int brightness) {
  if (brightness < 0 || brightness > 255) return;
  strip.setBrightness(brightness);
}

void setLedPixelBrightnessToPreset() {
  setLedPixelBrightness(desiredBrightness);
}

void lightCurrentPosition(int ledR, int ledG, int ledB) {
  int currentArmLedToLight = computeLedFromArmPosition(armPosition());
  setLedPixelRgbColor(currentArmLedToLight, ledR, ledG, ledB);
  if (computeLedOverlapTransitionFromArmPosition(armPosition())) 
      setLedPixelRgbColor(currentArmLedToLight + 1, ledR, ledG, ledB);
}

void showCurrentPosition() {
  strip.clear();
  lightCurrentPosition(baseRgb[RgbR], baseRgb[RgbG], baseRgb[RgbB]);
  setLedPixelBrightnessToPreset();
  strip.show();
}

void setupChaser(int from, int to) {
  chaserDirection = (from < to)? true : false;
  chaserRangeFrom = from;
  chaserRangeTo = to;
  currentChaserStep = from;
}

void runChaser() {
  // can't chase on a single led.
  if (numberOfPixels() <= 1) {
    showCurrentPosition();
    return; 
  }
  
    static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate < chaserSpeed) return;
  lastUpdate = millis();

  if (currentChaserStep == -1) currentChaserStep = chaserRangeFrom;

  strip.clear();

  setLedPixelRgbColor(chaserRangeFrom, 255, 69, 0);
  setLedPixelRgbColor(chaserRangeTo, 0, 255, 200);
  lightCurrentPosition(0, 32, 128);
  setLedPixelRgbColorToPreset(currentChaserStep);

  setLedPixelBrightnessToPreset();
  strip.show();

  if (chaserDirection) {
    if (currentChaserStep < (numberOfPixels() - 1)) currentChaserStep++;
    else currentChaserStep = chaserRangeFrom;
  } 
  else {
    if (currentChaserStep > 0) currentChaserStep--;
    else currentChaserStep = numberOfPixels() - 1;
  }
}

void turnOnLedPresets() {
  strip.clear();
  setLedPixelBrightnessToPreset();
  setLedPixelRgbColorToPreset();
  strip.show();
}

void turnOffLeds() {
  strip.clear();
  setLedPixelBrightness(0);
  strip.show();
}

void animateLeds() {
  maintainSetupValues();

  if (!ledPixelEnable) {
    turnOffLeds();
    return;
  }

  switch (animationStyle) {
    case NONE:
      if (isHome()) {
        turnOffLeds();
      }
      else showCurrentPosition();
      break;
    case WAKEPULSE:
      turnOnLedPresets();
      break;
    case ANIMHOME:  
    case MOVECHASER:  
      runChaser();
      break;
    case PULSEDETECT:  
      turnOnLedPresets(); 
      break;
    case UPHOME:  
    case UPMOVE:  
    case FOLLOWPLAY:  
      showCurrentPosition();
      break;
  }
}