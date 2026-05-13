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

volatile bool ledPixelEnable        = false;
volatile int  availablePixels       = 1;
volatile bool newLedPixelEnable     = false;
volatile int  newAvailablePixels    = 1;
volatile int  newChaserSpeed        = 1;
volatile int  newBreatherAdjustment = 1;
volatile int  newDesiredBrightness  = 1;

//these needs to be a 1:1 match for the turntable state machine
enum LedAnimation {NONE, WAKEPULSE, ANIMHOME, UPHOME, UPMOVE, MOVECHASER, PULSEDETECT, WARNREJECT, FOLLOWPLAY};
LedAnimation animationStyle = NONE;
enum RGB {RgbR, RgbG, RgbB};
int baseRgb[3] = {0, 0, 0};
volatile int desiredBrightness = 50;

//breather
volatile int breatherAdjustment = 50; 

//chaser
int chaserRangeFrom = 0;
int chaserRangeTo = 5;
volatile int chaserSpeed = 111; //for a 1sec animation 1000/number of pixels
bool chaserDirection = true;
static int currentChaserStep = -1;

bool setupStarted = false;

//=================================================================================================
void ledPixelSetup() {
  ledsetup.begin("ledsetup", false);
  loadLedPixelEnable();
  loadLedPixelNumber();
  loadChaserSpeed();
  loadBreatherAdjustment();
  loadDesiredBrightness();

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


void activateChaserSpeed(int spd) {
  chaserSpeed = spd;
}

void changeChaserSpeed(int spd) {
  newChaserSpeed = spd;
}

void storeChaserSpeed() {
  ledsetup.putInt("chaser", newChaserSpeed);
  activateChaserSpeed(newChaserSpeed);
}

void loadChaserSpeed() {
  chaserSpeed = ledsetup.getInt("chaser", 111);
  newChaserSpeed = chaserSpeed;
}


void activateBreatherAdjustment(int adj) {
  breatherAdjustment = adj;
}

void changeBreatherAdjustment(int adj) {
  newBreatherAdjustment = (adj < 1)? 1 : adj;
}

void storeBreatherAdjustment() {
  ledsetup.putInt("breather", newBreatherAdjustment);
  activateBreatherAdjustment(newBreatherAdjustment);
}

void loadBreatherAdjustment() {
  breatherAdjustment = ledsetup.getInt("breather", 250);
  newBreatherAdjustment = breatherAdjustment;
}

void activateDesiredBrightness(int brt) {
  desiredBrightness = brt;
}

void changeDesiredBrightness(int brt) {
  if (brt < 0) newDesiredBrightness = 0;
  else if (brt > 255) newDesiredBrightness = 255;
  else newDesiredBrightness = brt;
}

void storeDesiredBrightness() {
  ledsetup.putInt("brightness", newDesiredBrightness);
  activateDesiredBrightness(newDesiredBrightness);
}

void loadDesiredBrightness() {
  desiredBrightness = ledsetup.getInt("brightness", 50);
  newDesiredBrightness = desiredBrightness;
}

int numberOfPixels() {
  return availablePixels;
}

int chaserSpeedValue() {
  return chaserSpeed;
}

int breatherAdjValue() {
  return breatherAdjustment;
}

int ledBrightnessValue() {
  return desiredBrightness;
}

void maintainSetupValues() {
  if (newLedPixelEnable     != ledPixelEnable)     storeLedPixelEnable();
  if (newAvailablePixels    != availablePixels)    storeLedPixelNumber();
  if (newChaserSpeed        != chaserSpeed)        storeChaserSpeed();
  if (newBreatherAdjustment != breatherAdjustment) storeBreatherAdjustment();
  if (newDesiredBrightness  != desiredBrightness)  storeDesiredBrightness();
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

bool computeLedOverlapTransitionFromArmPosition(uint16_t stepToCompute) {
  float progress = (float)(stepToCompute * (numberOfPixels() - 1)) / getArmMaxValue();
  return (progress - (int)progress) > 0.5;
}

int computeLedFromArmPosition(uint16_t stepToCompute) {
  return (int)computeExactLedFromArmPosition(stepToCompute);
}

float computeExactLedFromArmPosition(uint16_t stepToCompute) {
  return (float)(stepToCompute * (numberOfPixels() - 1)) / getArmMaxValue();
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

void addLedPixelRgbColor(int ledIndex, int ledR, int ledG, int ledB) {
  if (ledR < 0 || ledR > 255 ||
      ledG < 0 || ledG > 255 ||
      ledB < 0 || ledB > 255 ||
      ledIndex < 0 || ledIndex >= numberOfPixels()) return;

  uint32_t currentPackedColor = strip.getPixelColor(ledIndex); 

  int currentR = (currentPackedColor >> 16) & 0xFF;
  int currentG = (currentPackedColor >> 8)  & 0xFF;
  int currentB = currentPackedColor         & 0xFF;

  int newR = currentR + ledR; if (newR > 255) newR = 255;
  int newG = currentG + ledG; if (newG > 255) newG = 255;
  int newB = currentB + ledB; if (newB > 255) newB = 255;

  setLedPixelRgbColor(ledIndex, newR, newG, newB);
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

void setLedPixelRgbColorToPreset(int ledIndex, float intensity) {
  setLedPixelRgbColor(ledIndex, baseRgb[RgbR] * intensity, baseRgb[RgbG] * intensity, baseRgb[RgbB] * intensity);
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

void addLightToCurrentPosition(int ledR, int ledG, int ledB) {
  float exactLedPosition = computeExactLedFromArmPosition(armPosition());
  int firstLed = (int)exactLedPosition;
  float fraction = exactLedPosition - firstLed;

  float intensityFirst = sqrt(1.0f - fraction);
  float intensitySecond = sqrt(fraction);

  addLedPixelRgbColor(firstLed, ledR * intensityFirst, ledG * intensityFirst, ledB * intensityFirst);

  int secondLed = firstLed + 1;
  if (secondLed < numberOfPixels()) {
    addLedPixelRgbColor(secondLed, ledR * intensitySecond, ledG * intensitySecond, ledB * intensitySecond);
  }
}

void lightCurrentPosition(int ledR, int ledG, int ledB) {
  float exactLedPosition = computeExactLedFromArmPosition(armPosition());
  
  int firstLed = (int)exactLedPosition;
  float fraction = exactLedPosition - firstLed;

  float intensityFirst = sqrt(1.0f - fraction);
  float intensitySecond = sqrt(fraction);

  setLedPixelRgbColor(firstLed, ledR * intensityFirst, ledG * intensityFirst, ledB * intensityFirst);

  int secondLed = firstLed + 1;
  if (secondLed < numberOfPixels()) {
    setLedPixelRgbColor(secondLed, ledR * intensitySecond, ledG * intensitySecond, ledB * intensitySecond);
  }
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

void runFlasher() {
  strip.clear();
  setLedPixelRgbColorToPreset();
  setLedPixelBrightness(((millis() % 333) < 166) ? desiredBrightness : 0);
  strip.show();
}

void runBreather() {
  strip.clear();
  setLedPixelRgbColorToPreset();
  setLedPixelBrightness((int)(((sin(millis() / (float)breatherAdjustment) + 1.0) / 2.0) * desiredBrightness));
  strip.show();
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

  float exactLedPosition = computeExactLedFromArmPosition(armPosition());

  float distFrom  = fabs(exactLedPosition - chaserRangeFrom);
  float distTo    = fabs(exactLedPosition - chaserRangeTo);
  float distChase = fabs(exactLedPosition - currentChaserStep);
  


  float fadeFrom  = distFrom < 1.0f ? distFrom : 1.0f;
  float fadeTo    = distTo < 1.0f ? distTo : 1.0f;
  float fadeChase = distChase < 1.0f ? distChase : 1.0f;


  setLedPixelRgbColor(chaserRangeFrom, 255 * fadeFrom, 69 * fadeFrom, 0);
  setLedPixelRgbColor(chaserRangeTo, 0 * fadeTo, 255 * fadeTo, 200);

  setLedPixelRgbColorToPreset(currentChaserStep, fadeChase);

  addLightToCurrentPosition(0, 32, 128);

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
  }
  else {
    switch (animationStyle) {
      case NONE:
        if (isHome()) {
          turnOffLeds();
        }
        else showCurrentPosition();
        break;
      case WAKEPULSE:
        runBreather();
        break;
      case ANIMHOME:  
      case MOVECHASER:  
        runChaser();
        break;
      case PULSEDETECT:  
        runBreather(); 
        break;
      case UPHOME:  
      case UPMOVE:  
      case FOLLOWPLAY:  
        showCurrentPosition();
        break;
      case WARNREJECT:
        runFlasher();
        break;
    }
  }
}