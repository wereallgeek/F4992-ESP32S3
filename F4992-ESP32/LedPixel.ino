#include <Adafruit_NeoPixel.h>
// NeoPixel Extension 
#define PIN_LEDPIXEL         48  
#define NUM_PIXELS           1  

Adafruit_NeoPixel strip(NUM_PIXELS, PIN_LEDPIXEL, NEO_GRB + NEO_KHZ800);
bool ledPixelEnable = true;

//these needs to be a 1:1 match for the turntable state machine
enum LedAnimation {NONE, WAKEPULSE, ANIMHOME, UPHOME, UPMOVE, ANIMRIGHT, PULSEDETECT, STEADYPLAY};
LedAnimation animationStyle = NONE;
enum RGB {RgbR, RgbG, RgbB};
int baseRgb[3] = {0, 0, 0};
int desiredBrightness = 50;

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
  if (style < NONE || style > STEADYPLAY) animationStyle = NONE;
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

void setAnimationMode(int style, const char* hexColor) {
  setAnimationStyle(style);
  setbaseRgb(hexColor);
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

void setLedPixelBrightness(int brightness) {
  if (brightness < 0 || brightness > 255) return;
  strip.setBrightness(brightness);
}

void setLedPixelBrightnessToPreset() {
  setLedPixelBrightness(desiredBrightness);
}

void animateLeds() {
  if (!ledPixelEnable) return;

  switch (animationStyle) {
    case NONE:
      setLedPixelBrightness(0);
      break;
    case WAKEPULSE:
      setLedPixelBrightnessToPreset();
      setLedPixelRgbColorToPreset();
      break;
    case ANIMHOME:  
    case ANIMRIGHT:  
      setLedPixelBrightnessToPreset();
      setLedPixelRgbColorToPreset();
      break;
    case UPHOME:  
    case UPMOVE:  
      setLedPixelBrightnessToPreset();
      setLedPixelRgbColorToPreset();
      break;
    case PULSEDETECT:  
      setLedPixelBrightnessToPreset();
      setLedPixelRgbColorToPreset();
      break;
    case STEADYPLAY:  
      setLedPixelBrightnessToPreset();
      setLedPixelRgbColorToPreset();
      break;
  }
  strip.show(); 
}