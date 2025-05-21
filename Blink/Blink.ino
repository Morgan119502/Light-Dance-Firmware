/// @file    Blink.ino
/// @brief   Blink the first LED of an LED strip
/// @example Blink.ino

#include <FastLED.h>
#include <WiFi.h>

// How many leds in your strip?
#define NUM_LEDS 10
// Define the array of leds
CRGB led1[NUM_LEDS];
CRGB led2[NUM_LEDS];
CRGB led3[NUM_LEDS];

CRGB* leds[3] = { led1, led2, led3 };

IPAddress local_IP(192, 168, 50, 101);

void setup() {
  // delay(5000);
  Serial.begin(9600);
  Serial.println("Connecting WiFi...");
  
  WiFi.setTimeout(2000);
  WiFi.config(local_IP);
  WiFi.begin("Lightdance", "wifiyee219");
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin("Lightdance", "wifiyee219");
  }

//   typedef enum {
//     WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
//     WL_IDLE_STATUS      = 0,
//     WL_NO_SSID_AVAIL    = 1,
//     WL_SCAN_COMPLETED   = 2,
//     WL_CONNECTED        = 3,
//     WL_CONNECT_FAILED   = 4,
//     WL_CONNECTION_LOST  = 5,
//     WL_DISCONNECTED     = 6
// } wl_status_t;

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
  delay(1000);
    for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 10; j++) {
      leds[i][j] = CRGB::Blue;
    }
  }
  
  FastLED.show();
  delay(1000);
}
