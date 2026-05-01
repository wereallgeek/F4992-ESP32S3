#include <Adafruit_NeoPixel.h>
// NeoPixel Extension 
#define PIN_LEDPIXEL         48  
#define NUM_PIXELS           1  

Adafruit_NeoPixel strip(NUM_PIXELS, PIN_LEDPIXEL, NEO_GRB + NEO_KHZ800);
bool ledPixelEnable = true;

void setLedPixelEnable(bool enable) {
  ledPixelEnable = enable;
}

bool useLedPixel() {
  return ledPixelEnable;
}

void ledPixelSetup() {
  strip.begin();
  setLedPixelBrightness(ledPixelEnable ? 50 : 0);
  strip.show(); 
}

void setLedPixelHexColor(int ledIndex, const char* hexColor) {
  int ledR, ledG, ledB;
  if (sscanf(hexColor, "#%02x%02x%02x", &ledR, &ledG, &ledB) == 3) {
    setLedPixelRgbColor(ledIndex, ledR, ledG, ledB);
  }
}

void setLedPixelHexColor(const char* hexColor) {
  setLedPixelHexColor(0, hexColor);
}

void setLedPixelRgbColor(int ledIndex, int ledR, int ledG, int ledB) {
  if (ledR < 0 || ledR > 255 ||
      ledG < 0 || ledG > 255 ||
      ledB < 0 || ledB > 255 ||
      ledIndex < 0 || ledIndex >= NUM_PIXELS) return;
  strip.setPixelColor(ledIndex, strip.Color(ledR, ledG, ledB));
}

void setLedPixelBrightness(int brightness) {
  if (brightness < 0 || brightness > 255) return;
  strip.setBrightness(brightness);
}

void animateLeds() {
  if (!ledPixelEnable) return;
  //here add the animations
  strip.show(); 
}