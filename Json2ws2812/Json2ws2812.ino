#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <FastLED.h>
#include <EasyButton.h>

const char* ssid = "EE219B";          // wifi名稱
const char* password = "wifiyee219";  // wifi密碼

#define switch 17
#define btnpin 16
#define CNT 4096
#define CHUNK_SIZE 100

int ledPins[7] = { 2, 3, 4, 5, 6, 7, 8 };
CRGB led1[1];
CRGB led2[1];
CRGB led3[1];
CRGB led4[1];
CRGB led5[1];
CRGB led6[1];
CRGB led7[1];
unsigned long startTime = 0;
CRGB* leds[7] = { led1, led2, led3, led4, led5, led6, led7 };
unsigned int array[CNT][8];
EasyButton btn1(btnpin, 100, true);
bool ON = 0;
bool wifisw = 0;
int i = 0;
String memory;  //光表

void wifiConnect() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void onButton() {
  ON = !ON;
  if (ON) {
    startTime = millis();
    i = 0;  //按了按鈕後是要從頭開始還是接著
  }
}

void fetch(int chunk) {
  HTTPClient http;
  String apiurl = "http://140.113.160.136:8000/get_test_lightlist/cnt=";
  String cnt_str = String(CNT);
  String mid_str = "/chunk=";
  String chunk_str = String(chunk);
  apiurl = apiurl + cnt_str + mid_str + chunk_str;

  http.begin(apiurl);
  JsonDocument doc;

  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    memory = http.getString();
    Serial.println("api success");
    Serial.println(apiurl);
    Serial.print("chunk = ");
    Serial.println(chunk_str);
    Serial.println("data:");
    Serial.println(memory);
  } else {
    Serial.println("api failed");
    Serial.printf("HTTP request failed, error code: %d\n", httpResponseCode);
    Serial.println(http.errorToString(httpResponseCode));  // 詳細的錯誤描述
  }

  File writefile = LittleFS.open("/lightlist.json", "w");
  if (writefile) {
    writefile.println(memory);
    writefile.close();  // 關閉檔案
    Serial.println("Success writing");
  } else {
    Serial.println("Failed to open file for writing");
  }

  //開啟記憶體中的光表
  File readfile = LittleFS.open("/lightlist.json", "r");
  if (readfile && readfile.size() > 0) {
    Serial.println("memoryfile exist");
    // 若檔案存在，解析 JSON
    DeserializationError error = deserializeJson(doc, readfile);
    readfile.close();

    if (!error) {
      Serial.println("Success to parse JSON");
    } else {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.println("File not found, creating new memoryfile");
    // 檔案不存在，建立新檔案
    File newFile = LittleFS.open("/lightlist.json", "w");
    if (newFile) {
      newFile.println("{}");  // 建立空的 JSON 結構
      newFile.close();
      Serial.println("New memoryfile created");
    } else {
      Serial.println("Failed to create new memoryfile");
    }
  }
  
  for (int i = 0; i < min(CHUNK_SIZE, CNT-CHUNK_SIZE*chunk); i++) {
    array[i+CHUNK_SIZE*chunk][0] = doc["color_data"][i]["time"];
    array[i+CHUNK_SIZE*chunk][1] = doc["color_data"][i]["head"];
    array[i+CHUNK_SIZE*chunk][2] = doc["color_data"][i]["shoulder"];
    array[i+CHUNK_SIZE*chunk][3] = doc["color_data"][i]["chest"];
    array[i+CHUNK_SIZE*chunk][4] = doc["color_data"][i]["arm_waist"];
    array[i+CHUNK_SIZE*chunk][5] = doc["color_data"][i]["leg1"];
    array[i+CHUNK_SIZE*chunk][6] = doc["color_data"][i]["leg2"];
    array[i+CHUNK_SIZE*chunk][7] = doc["color_data"][i]["shoes"];
  }
}


void setup() {
  //fastled腳位宣告
  FastLED.addLeds<NEOPIXEL, 2>(led1, 1);
  FastLED.addLeds<NEOPIXEL, 3>(led2, 1);
  FastLED.addLeds<NEOPIXEL, 4>(led3, 1);
  FastLED.addLeds<NEOPIXEL, 5>(led4, 1);
  FastLED.addLeds<NEOPIXEL, 6>(led5, 1);
  FastLED.addLeds<NEOPIXEL, 7>(led6, 1);
  FastLED.addLeds<NEOPIXEL, 8>(led7, 1);
  pinMode(switch, INPUT_PULLUP);  //wifi mode & memory mode切換腳位
  // HTTPClient http;
  //淨空
  FastLED.clear();
  FastLED.show();

  delay(500);
  Serial.begin(115200);  // 啟動序列埠的連接
  while (!Serial)
    continue;
  //連接wifi
  wifiConnect();
  //Serial setting
  Serial.println("Starting setup...");
  if (!LittleFS.begin()) {
    Serial.println("LittleFS initialization failed. Formatting...");
    if (LittleFS.format()) {
      Serial.println("Formatting successful! Trying to initialize again...");
      if (LittleFS.begin()) {
        Serial.println("LittleFS initialized successfully.");
      } else {
        Serial.println("LittleFS initialization failed after formatting.");
      }
    } else {
      Serial.println("Formatting failed!");
    }
  } else {
    Serial.println("LittleFS initialized successfully.");
  }
  Serial.println("in setup");

  // //更新光表
  wifisw = digitalRead(switch);
  if (wifisw) {
    Serial.println("Wifi Mode");
    for (int i = 0; i < 7; i++) {
      leds[i][0] = CRGB::Red;
      delay(500);
    }
    //處理光表輸入
    //連接api
    for(int k=0; k < double(CNT)/double(CHUNK_SIZE); k++) {
      fetch(k);
    }    
  } else {
    Serial.println("Memory Mode");
    for (int i = 0; i < 7; i++) {
      leds[i][0] = CRGB::Green;
    }
  }

  FastLED.show();
  delay(500);
  FastLED.clear();
  FastLED.show();

  startTime = millis();
  btn1.begin();
  btn1.onPressed(onButton);
  pinMode(btnpin, INPUT_PULLUP);

  //檢查容量
  FSInfo fs_info;
  LittleFS.info(fs_info);

  // 顯示檔案系統的總容量和可用空間
  Serial.print("Total space: ");
  Serial.println(fs_info.totalBytes);

  Serial.print("Used space: ");
  Serial.println(fs_info.usedBytes);

  Serial.print("Free space: ");
  Serial.println(fs_info.totalBytes - fs_info.usedBytes);
}

int bright(unsigned int data) {
  return (data >> 0) & 0xFF;
}

void loop() {
  btn1.read();
  if (ON) {
    if (i < CNT && (millis() - startTime >= array[i][0] * 50)) {
      for (int j = 0; j < 7; j++) {
        leds[j][0] = array[i][j + 1] >> 8;
        leds[j][0].nscale8(bright(array[i][j + 1]));
        //Serial.printf("(%d, %d, %d)", red(array[i][j + 1]), green(array[i][j + 1]), blue(array[i][j + 1]));
      }
      i++;
      FastLED.show();
    }
  }
}