#include <Adafruit_NeoPixel.h>
// NeoPixel Extension 
#define PIN_LEDPIXEL         48  
#define NUM_PIXELS           9  //todo: expose to GUI

Adafruit_NeoPixel strip(NUM_PIXELS, PIN_LEDPIXEL, NEO_GRB + NEO_KHZ800);
bool ledPixelEnable = true; //todo: expose to GUI

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
int chaserSpeed = 111; //for a 1sec animation 1000/NUM_PIXELS
bool chaserDirection = true;
static int currentChaserStep = -1;

//=================================================================================================
void ledPixelSetup() {
  strip.begin();
  setLedPixelBrightness(ledPixelEnable ? desiredBrightness : 0);
  strip.show(); 
}

void setLedPixelEnable(bool enable) {
  ledPixelEnable = enable;
}

bool useLedPixel() {
  return ledPixelEnable;
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

int computeLedFromArmPosition(uint16_t stepToCompute) {
  return (stepToCompute * (NUM_PIXELS - 1)) / getArmMaxValue();
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
  for (int ledIndex = 0; ledIndex < NUM_PIXELS; ledIndex++) {
    setLedPixelHexColor(ledIndex, hexColor);
  }
}

void setLedPixelRgbColor(int ledIndex, int ledR, int ledG, int ledB) {
  if (ledR < 0 || ledR > 255 ||
      ledG < 0 || ledG > 255 ||
      ledB < 0 || ledB > 255 ||
      ledIndex < 0 || ledIndex >= NUM_PIXELS) return;
  strip.setPixelColor(ledIndex, strip.Color(ledR, ledG, ledB));
}


void setLedPixelRgbColor(int ledR, int ledG, int ledB) {
  for (int ledIndex = 0; ledIndex < NUM_PIXELS; ledIndex++) {
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
  setLedPixelRgbColor(computeLedFromArmPosition(armPosition()), ledR, ledG, ledB);
}

void showCurrentPosition() {
  strip.clear();
  lightCurrentPosition(0, 64, 192);
  strip.show();
}

void setupChaser(int from, int to) {
  chaserDirection = (from < to)? true : false;
  chaserRangeFrom = from;
  chaserRangeTo = to;
  currentChaserStep = from;
}

void runChaser() {
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate < chaserSpeed) return;
  lastUpdate = millis();

  if (currentChaserStep == -1) currentChaserStep = chaserRangeFrom;

  strip.clear();

  setLedPixelRgbColor(chaserRangeFrom, 255, 69, 0);
  setLedPixelRgbColor(chaserRangeTo, 0, 255, 200);
  lightCurrentPosition(0, 32, 128);
  setLedPixelRgbColorToPreset(currentChaserStep);

  strip.show();

  if (chaserDirection) {
    if (currentChaserStep < (NUM_PIXELS - 1)) currentChaserStep++;
    else currentChaserStep = chaserRangeFrom;
  } 
  else {
    if (currentChaserStep > 0) currentChaserStep--;
    else currentChaserStep = NUM_PIXELS - 1;
  }
}


void animateLeds() {
  if (!ledPixelEnable) return;

  switch (animationStyle) {
    case NONE:
      if (isHome()) setLedPixelBrightness(0);
      else showCurrentPosition();
      break;
    case WAKEPULSE:
      setLedPixelBrightnessToPreset();
      setLedPixelRgbColorToPreset();
      break;
    case ANIMHOME:  
    case MOVECHASER:  
      runChaser();
      break;
    case PULSEDETECT:  
      setLedPixelBrightnessToPreset();
      setLedPixelRgbColorToPreset();
      break;
    case UPHOME:  
    case UPMOVE:  
    case FOLLOWPLAY:  
      showCurrentPosition();
      break;
  }
  strip.show(); 
}