#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FastLED.h>
#include <EasyButton.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define SDA_PIN 12
#define SCL_PIN 13
#define btnpin 16
#define switch 17

const int ledPins[6] = { 2, 3, 4, 5, 6, 7 };
CRGB led1[5];   //頭
CRGB led2[4];   //肩
CRGB led3[11];  //胸、手
CRGB led4[4];   //腰、裙
CRGB led5[5];   //腿
CRGB led6[3];   //前
const int sectionSizes[] =    { 5, 4, 4, 5, 8, 11, 4, 4, 5, 3 };
int sectionStart[] =          { 0, 0, 0, 4, 5, 8, 0, 0, 4, 0 };
const int sectionIndices[] =  { 1, 2, 3, 4, 8, 9, 5, 6, 7, 4 };
const int sectionRows[] =     { 0, 1, 2, 2, 2, 2, 3, 4, 4, 5 };
bool wifisw = 0;
CRGB* leds[6] = { led1, led2, led3, led4, led5, led6 };
EasyButton btn1(btnpin, 100, true);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
bool ON = 0;
#define head     0xFF3B30  // RED
#define shoulder 0xFF9500  // ORANGE
#define chest    0xFFD60A  // YELLOW
#define front    0x64DD17  // LIME GREEN
#define skirt    0x00E676  // NEON GREEN
#define leg      0x40E0D0  // AQUA
#define shoes    0x5AC8FA  // SKY BLUE
#define weap_1   0xAF52DE  // PURPLE
#define weap_2   0xFF2D55  // PINK

unsigned int array[10] = { 0, head, shoulder, chest, front, skirt, leg, shoes, weap_1, weap_2};

void setup() {
  //FastLED.setBrightness(50);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  FastLED.addLeds<NEOPIXEL, 2>(led1, 5);
  FastLED.addLeds<NEOPIXEL, 3>(led2, 4);
  FastLED.addLeds<NEOPIXEL, 4>(led3, 11);
  FastLED.addLeds<NEOPIXEL, 5>(led4, 4);
  FastLED.addLeds<NEOPIXEL, 6>(led5, 5);
  FastLED.addLeds<NEOPIXEL, 7>(led6, 3);
  // FastLED.addLeds<NEOPIXEL, 8>(led7, 1);
  pinMode(switch, INPUT_PULLUP);
  FastLED.clear();
  wifisw = digitalRead(switch);
  if (wifisw) {
    Serial.println("Wifi Mode");
    for (int i = 0; i < 6; i++) {
      leds[i][0] = CRGB::Red;
    }
  } else {
    Serial.println("Memory Mode");
    for (int i = 0; i < 6; i++) {
      leds[i][0] = CRGB::Green;
    }
  }
  FastLED.show();
  Wire.setSDA(SDA_PIN);  // Set SDA to GP20
  Wire.setSCL(SCL_PIN);  // Set SCL to GP21
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.clearDisplay();
  display.setTextSize(2);  // Text size
  display.setTextColor(WHITE);
  display.setCursor(10, 25);
  display.print("Hello");
  display.display();
  btn1.begin();
  btn1.onPressed(onButton);
  pinMode(btnpin, INPUT_PULLUP);
}

void onButton() {
  ON = !ON;
  if (ON) {
    display.clearDisplay();
    display.setCursor(10, 25);
    display.print("btl");
    display.display();
  } else {
    display.clearDisplay();
    display.setCursor(10, 25);
    display.print("ahhhhh");
    display.display();
  }
}

void loop() {
  btn1.read();
  for (int i = 0; i < 10; i++) {
    for (int j = sectionStart[i]; j < sectionSizes[i]; j++) {
      leds[sectionRows[i]][j] = array[sectionIndices[i]];
    }
  }
  FastLED.show();

}
