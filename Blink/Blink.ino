/// @file    Blink.ino
/// @brief   Blink the first LED of an LED strip
/// @example Blink.ino

#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 10
// Define the array of leds
CRGB led1[NUM_LEDS];
CRGB led2[NUM_LEDS];
CRGB led3[NUM_LEDS];

CRGB* leds[3] = { led1, led2, led3 };

void setup() {
  FastLED.addLeds<NEOPIXEL, 14>(led1, NUM_LEDS);  // GRB ordering is assumed
  FastLED.addLeds<NEOPIXEL, 16>(led2, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, 21>(led3, NUM_LEDS);
  FastLED.setBrightness(255);
}

void loop() {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 10; j++) {
      leds[i][j] = CRGB::Red;
    }
  }
  FastLED.show();
}
