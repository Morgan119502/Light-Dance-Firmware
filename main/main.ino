#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <FastLED.h>
#include <EasyButton.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <SPI.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SDA_PIN 12
#define SCL_PIN 13

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiUDP udp;  // 建立 UDP 對象

unsigned int localPort = 12345;                 // 接收廣播的埠
const char* responseAddress = "192.168.0.189";  // 替換為 Python 廣播端的 IP 地址
unsigned int responsePort = 12346;              // 回傳訊息的埠

// WiFi 設定
const char* ssid = "EE219B";          // wifi名稱
const char* password = "wifiyee219";  // wifi密碼

// API設定
const char* serverUrl = "http://192.168.0.189:8000/api/bootcount";  // 請替換成你的API端點
const char* testUrl = "http://192.168.0.189:8000/health";
const char* remoteUrl = "http://140.113.160.136:8000/items/eesa1/2024-Oct-16-17:48:58";  //最後不要加斜線!!!!  // 可以用這個練字串處理了 OuOb
//const char* remoteUrl = "http://140.113.160.136:8000/timelist/";

// 全域變數
WiFiServer server(80);          // 設置 HTTP 伺服器埠
bool startMainProgram = false;  // 主程式啟動開關
bool running = false;           // 模擬任務執行狀態
bool tryToRcv = true;           // 是否嘗試接收檔案
String deviceId = "test02";     // 裝置名稱
bool firstStart = true;
int offset = 0;

// LED腳位設定
#define SWITCH_PIN 17
#define BUTTON_PIN 16
#define CNT 4096
#define CHUNK_SIZE 100
#define LED_COUNT 7

// LED setup
const int ledPins[LED_COUNT] = { 2, 3, 4, 5, 6, 7 };
CRGB leds[LED_COUNT][1];
unsigned int array[CNT][8];
EasyButton btn1(BUTTON_PIN, 100, true);

unsigned long startTime = 0;
bool ON = 0;
bool wifiMode = false;

// int i = 0;
int currentIndex = 0;
String memoryData;

// wifi連線
void connectToWiFi() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();
  WiFi.begin(ssid, password);
  Serial.print("connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());  // 印出 IP 位址

  // 更新 OLED 顯示成功
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected");
  display.setCursor(0, 16);
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();

  return;
}

void fetchChunk(int chunk) {
  HTTPClient http;
  String apiUrl = "http://140.113.160.136:8000/get_test_lightlist/cnt=" + String(CNT) + "/chunk=" + String(chunk);

  http.begin(apiUrl);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    memoryData = http.getString();
    // Serial.println(apiUrl);
    // Serial.println("API fetch successful");

    // File file;
    
    // if (chunk == 0) {
    //   file = LittleFS.open("/lightlist.json", "w");
    // } else {
    //   file = LittleFS.open("/lightlist.json", "a");
    //   if(file){
    //     Serial.println("open success");
    //     // String checkstring = file.readString();
    //     // Serial.println(checkstring);
    //   }else{
    //     Serial.println("open failed");
    //   }
      
    // }

    // if (file) {
    //   //file.seek(file.size());
    //   file.print(memoryData);
    //   file.close();
    //   Serial.println("Data saved to memory");
    // } else {
    //   Serial.println("Failed to save data to memory");
    // }

    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, memoryData);

    if (error) {
      Serial.print("JSON data size: ");
      Serial.println(strlen(memoryData.c_str()));
      Serial.print("JSON parsing error: ");
      Serial.println(error.c_str());
      return;
    }else{
      Serial.println("deserialization success");
    }

    for (int i = 0; i < min(CHUNK_SIZE, CNT - CHUNK_SIZE * chunk); i++) {
      array[i + CHUNK_SIZE * chunk][0] = doc["color_data"][i]["time"];
      array[i + CHUNK_SIZE * chunk][1] = doc["color_data"][i]["head"];
      array[i + CHUNK_SIZE * chunk][2] = doc["color_data"][i]["shoulder"];
      array[i + CHUNK_SIZE * chunk][3] = doc["color_data"][i]["chest"];
      array[i + CHUNK_SIZE * chunk][4] = doc["color_data"][i]["arm_waist"];
      array[i + CHUNK_SIZE * chunk][5] = doc["color_data"][i]["leg1"];
      array[i + CHUNK_SIZE * chunk][6] = doc["color_data"][i]["leg2"];
      array[i + CHUNK_SIZE * chunk][7] = doc["color_data"][i]["shoes"];
    }

  } else {
    Serial.printf("API fetch failed with error code: %d\n", httpResponseCode);
  }
  http.end();
}

// Load data from memory
// void loadLightListFromMemory() {
//   File file = LittleFS.open("/lightlist.json", "r");
//   if (file && file.size() > 0) {
//     Serial.println("Read data in memory");
//     memoryData = file.readString();
//     Serial.println(memoryData);
//     file.close();

//     StaticJsonDocument<4096> doc;
//     DeserializationError error = deserializeJson(doc, memoryData);

//     if (error) {
//       Serial.print("JSON parsing error: ");
//       Serial.println(error.c_str());
//       return;
//     }

//     for (int i = 0; i < CNT; i++) {
//       array[i][0] = doc["color_data"][i]["time"];
//       array[i][1] = doc["color_data"][i]["head"];
//       array[i][2] = doc["color_data"][i]["shoulder"];
//       array[i][3] = doc["color_data"][i]["chest"];
//       array[i][4] = doc["color_data"][i]["arm_waist"];
//       array[i][5] = doc["color_data"][i]["leg1"];
//       array[i][6] = doc["color_data"][i]["leg2"];
//       array[i][7] = doc["color_data"][i]["shoes"];
//     }

//   } else {
//     Serial.println("No data found in memory");
//   }
// }
// void loadLightListFromMemory() {
//     File file = LittleFS.open("/lightlist.json", "r");
//     if (!file || file.size() == 0) {
//         Serial.println("No data found in memory");
//         return;
//     }

//     const size_t bufferSize = 512; // 單次讀取的緩衝區大小
//     char buffer[bufferSize + 1];   // +1 用於 null 結尾
//     String incompletePart = "";  // 用於儲存上一段未完成的部分
//     size_t offset = 0;            // 用於記錄檔案讀取的偏移量

//     while (offset < file.size()) {
//         file.seek(offset, SeekSet); // 設定檔案偏移量
//         size_t bytesRead = file.readBytes(buffer, bufferSize);
//         buffer[bytesRead] = '\0'; // 確保字串結尾
//         offset += bytesRead;       // 更新偏移量

//         // 將未完成部分與當前段結合
//         String jsonChunk = incompletePart + String(buffer);

//         // 嘗試解析完整 JSON
//         StaticJsonDocument<1024> doc; // 根據實際 JSON 結構調整大小
//         DeserializationError error = deserializeJson(doc, jsonChunk);

//         if (error) {
//             if (error == DeserializationError::IncompleteInput) {
//                 // 若解析失敗但為不完整輸入，保留當前段作為未完成部分
//                 incompletePart = jsonChunk;
//             } else {
//                 Serial.print("JSON parsing error: ");
//                 Serial.println(error.c_str());
//                 file.close();
//                 return;
//             }
//         } else {
//             incompletePart = ""; // 清空未完成部分

//             // 確保 color_data 是陣列
//             if (!doc["color_data"].is<JsonArray>()) {
//                 Serial.println("color_data is not an array");
//                 continue;
//             }

//             JsonArray colorData = doc["color_data"].as<JsonArray>();

//             // 遍歷 JSON 陣列
//             for (JsonObject obj : colorData) {
//                 int index = obj["index"].as<int>(); // 根據 JSON 結構調整
//                 if (index >= 0 && index < CNT) {
//                     array[index][0] = obj["time"].as<int>();
//                     array[index][1] = obj["head"].as<int>();
//                     array[index][2] = obj["shoulder"].as<int>();
//                     array[index][3] = obj["chest"].as<int>();
//                     array[index][4] = obj["arm_waist"].as<int>();
//                     array[index][5] = obj["leg1"].as<int>();
//                     array[index][6] = obj["leg2"].as<int>();
//                     array[index][7] = obj["shoes"].as<int>();
//                 }
//             }
//         }
//     }

//     if (incompletePart.length() == 0) {
//         Serial.println("Warning: Incomplete JSON data at the end of the file");
//     }

//     file.close();
// }



// Setup for Wi-Fi mode
void setupWiFiMode() {
  connectToWiFi();
  for (int chunk = 0; chunk < (CNT + CHUNK_SIZE - 1) / CHUNK_SIZE; chunk++) {
    fetchChunk(chunk);
  }
  saveArrayToFile();
}

void saveArrayToFile() {
    File file = LittleFS.open("/array_data.bin", "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    for (int i = 0; i < CNT; ++i) {
        for (int j = 0; j < 8; ++j) {
            file.write((uint8_t *)&array[i][j], sizeof(unsigned int));
        }
    }

    file.close();
    Serial.println("Array data saved to file");
}
void loadArrayFromFile() {
    File file = LittleFS.open("/array_data.bin", "r");
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    for (int i = 0; i < CNT; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (file.available()) {
                file.read((uint8_t *)&array[i][j], sizeof(unsigned int));
            } else {
                Serial.println("File does not contain enough data");
                file.close();
                return;
            }
        }
    }

    file.close();
    Serial.println("Array data loaded from file");
}


// Setup for Memory mode
void setupMemoryMode() {
      loadArrayFromFile();
}

// Brightness calculation
int calculateBrightness(unsigned int data) {
  return (data >> 0) & 0xFF;
}

void onButton() {
  ON = !ON;
  startMainProgram = !startMainProgram;
  Serial.println(ON);
  if (ON) {
    startTime = millis();
    currentIndex = 0;  //按了按鈕後是要從頭開始還是接著
  }
}

// test get api
void getCheck() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    Serial.print("Connecting to server: ");
    Serial.println(testUrl);

    http.begin(client, testUrl);
    //    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      Serial.println("GET request successful!");
      String response = http.getString();
      Serial.print("Response size: ");
      Serial.println(http.getSize());
      Serial.print("Response: ");
      Serial.println(response);

    } else {
      Serial.print("GET request failed, error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
}

void remoteCheck() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    Serial.print("Connecting to server: ");
    Serial.println(remoteUrl);

    http.begin(client, remoteUrl);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      Serial.println("remote GET request successful!");
      String response = http.getString();
      Serial.print("remote Response size: ");
      Serial.println(http.getSize());
      Serial.print("remote Response: ");
      Serial.println(response);

    } else {
      Serial.print("remote GET request failed, error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
}

// 發送數據到伺服器的函數
bool sendDataToServer(int bootCount) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    Serial.print("Connecting to server: ");
    Serial.println(serverUrl);

    // 開始連接
    if (http.begin(client, serverUrl)) {
      http.addHeader("Content-Type", "application/json");

      // 準備JSON數據
      StaticJsonDocument<200> doc;  // 使用StaticJsonDocument來避免記憶體問題
      doc["bootCount"] = bootCount;
      doc["deviceId"] = deviceId;

      String jsonString;
      serializeJson(doc, jsonString);

      Serial.print("Sending data: ");
      Serial.println(jsonString);

      // 發送POST請求
      int httpResponseCode = http.POST(jsonString);

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        http.end();
        return true;
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
        Serial.println(http.errorToString(httpResponseCode));
        http.end();
        return false;
      }
    } else {
      Serial.println("Unable to connect to server");
      return false;
    }
  } else {
    Serial.println("Error in WiFi connection");
    return false;
  }
}

void checkHTTP() {
  // 處理 HTTP 請求
  WiFiClient client = server.available();
  if (client) {
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    bool GotStart = (request.indexOf("start") != -1);

    // 檢查是否為 /start API
    if (GotStart) {
      Serial.println("Got signal from URL!");
      startMainProgram = true;  // 啟動主程式
      ON = true;
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");  // 指定回應類型為 HTML
      client.println("Connection: close");        // 告訴瀏覽器關閉連接
      client.println();
      client.println("<!DOCTYPE html>");
      client.println("<html>");
      client.println("<head><title>Pico W</title></head>");
      client.println("<body>");
      client.println("<h1>Program active</h1>");
      client.println("<p>Signal received and program started!</p>");
      client.println("</body>");
      client.println("</html>");
    }

    delay(10);
    client.stop();
  }
}

void checkUDP() {
  // 新增：接收 UDP 廣播訊息
  char incomingPacket[255];  // 用於存儲接收的廣播消息
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    incomingPacket[len] = 0;  // 確保字串以 '\0' 結尾
    String command = String(incomingPacket);
    if (command != "heartbeat") Serial.printf("Received UDP packet: %s\n", command.c_str());
    handleCommand(command);  // 處理接收到的指令
  }
}

int checkUDP_number() {
  // 檢查是否有 UDP 資料
  int packetSize = udp.parsePacket();
  if (packetSize) {
    handleCommand("number");
    // 讀取資料
    byte buffer[4];
    udp.read(buffer, 4);  // 假設廣播的資料是 4-byte 的整數
    // 將資料解碼為整數
    uint32_t receivedNumber = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
    if (receivedNumber == 1937010544) {  // stop
      handleCommand("stop");
      startMainProgram = false;
      ON = false;
      return -1;
    }
    if (receivedNumber == 1937006962) {  // start
      handleCommand("start");
      startMainProgram = true;
      currentIndex = 0;
      ON = true;
      firstStart = false;
      offset = 0;
      return -2;
    }
    if (receivedNumber == 1751474546) {  // start
      handleCommand("heartbeat");
      return -2;
    }
    // Serial.println(receivedNumber);
    return receivedNumber;
  }
  return 0;
}

// 處理 UDP 指令
void handleCommand(String command) {
  if (command == "start") {
    startMainProgram = true;
    ON = true;
    running = true;
    Serial.println("Received 'start' command.");
    String response = deviceId + ": running";
    udp.beginPacket(responseAddress, responsePort);
    udp.write(response.c_str());
    udp.endPacket();
  } else if (command == "stop") {
    startMainProgram = false;
    ON = false;
    running = false;
    Serial.println("Received 'stop' command.");
    String response = deviceId + ": stopped";
    udp.beginPacket(responseAddress, responsePort);
    udp.write(response.c_str());
    udp.endPacket();
  } else if (command == "heartbeat") {
    //Serial.println("Received 'heartbeat' command.");
    String response = deviceId + ": heartbeat received";
    udp.beginPacket(responseAddress, responsePort);
    udp.write(response.c_str());
    udp.endPacket();
  } else if (command == "number") {
    String response = deviceId + ": number received";
    udp.beginPacket(responseAddress, responsePort);
    udp.write(response.c_str());
    udp.endPacket();
  } else {
    Serial.println("Unknown command: " + command);
  }
}

void testmain() {
  // 新增：接收 UDP 廣播訊息
  char incomingPacket[255];  // 用於存儲接收的廣播消息
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    incomingPacket[len] = 0;  // 確保字串以 '\0' 結尾
    String command = String(incomingPacket);
    if (command != "heartbeat") Serial.printf("Received UDP packet: %s\n", command.c_str());
    handleCommand(command);  // 處理接收到的指令
  }
  int bootCount = 0;
  File file = LittleFS.open("/bootCount.json", "r");
  if (file) {
    StaticJsonDocument<200> doc;  // 使用StaticJsonDocument來避免記憶體問題
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (!error) {
      bootCount = doc["bootCount"] | 0;
      Serial.printf("Boot count read from file: %d\n", bootCount);
    } else {
      Serial.println("Failed to parse JSON, starting with bootCount = 0");
    }
  } else {
    Serial.println("File not found, creating new one with bootCount = 0");
  }

  // 增加開機次數並更新 JSON 數據
  bootCount++;
  StaticJsonDocument<200> doc;
  doc["bootCount"] = bootCount;

  // 打開檔案以寫入新的 JSON 數據
  file = LittleFS.open("/bootCount.json", "w");
  if (file) {
    serializeJson(doc, file);
    file.close();
    Serial.printf("Updated boot count: %d\n", bootCount);

    // 嘗試發送數據到伺服器，如果失敗則重試
    int retryCount = 0;
    while (!sendDataToServer(bootCount) && retryCount < 3) {
      Serial.println("Retrying...");
      delay(1000);
      retryCount++;
    }
  } else {
    Serial.println("Failed to open file for writing");
  }
}

void tryRcv() {
}

unsigned long currentTime = 0;
void mainProgram() {  // 照著光表亮
  Serial.println("enter main");
  while (1) {
    Serial.println("enter loop");
    if (ON) {
      Serial.println("on");
      startTime = millis();
      while (currentIndex < CNT) {
        // Serial.print(".");
        btn1.read();
        int ii = checkUDP_number();
        if (ii == -1) return;
        if (ii > 0) {
          // Serial.print(ii);
          currentIndex = ii / 1000 * 20;
          Serial.print("currentIndex: ");
          Serial.println(currentIndex);
          if (firstStart) {
            firstStart = false;
            startTime = millis();
            offset = currentIndex;
            Serial.print("offset: ");
            Serial.println(offset);
          }
        }
        // Serial.print("currentIndex: ");
        // Serial.println(currentIndex);
        // Serial.print(currentIndex);
        // Serial.print(" ");
        // Serial.print(millis() - startTime);
        // Serial.print(" ");
        // Serial.println((currentIndex - offset) * 50);
        if (!firstStart and millis() - startTime >= (currentIndex - offset) * 50) {
          // Serial.println("print");
          for (int j = 0; j < LED_COUNT; j++) {
            leds[j][0] = array[currentIndex][j + 1] >> 8;
            //Serial.print(array[currentIndex][j + 1]);
            leds[j][0].nscale8(calculateBrightness(array[currentIndex][j + 1]));
          }
          currentIndex++;
          FastLED.show();
        }
      }
    }
  }
  Serial.println("out main");
}

int bright(unsigned int data) {
  return (data >> 0) & 0xFF;
}

void setup() {
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  delay(1000);

  /*
  display.setCursor(0, 0);    // 可刪掉，馬上會被刷新所以顯示不出來
  display.println("Start.");  // 我不知道為啥上面那些東西一定要放在 delay 上面才能跑，然後少一個都不行
  display.display();          // 就很玄  
  */

  Serial.println("Start.");
  Serial.begin(115200);

  delay(1000);

  while (!Serial) {}
  // 連接 WiFi
  connectToWiFi();

  if (!LittleFS.begin()) {
    Serial.println("Failed to initialize LittleFS");
    return;
  }

  // 啟動 HTTP 伺服器
  server.begin();

  if (tryToRcv) tryRcv();

  // Initialize LED
  FastLED.addLeds<NEOPIXEL, 2>(leds[0], 1);
  FastLED.addLeds<NEOPIXEL, 3>(leds[1], 1);
  FastLED.addLeds<NEOPIXEL, 4>(leds[2], 1);
  FastLED.addLeds<NEOPIXEL, 5>(leds[3], 1);
  FastLED.addLeds<NEOPIXEL, 6>(leds[4], 1);
  FastLED.addLeds<NEOPIXEL, 7>(leds[5], 1);
  FastLED.addLeds<NEOPIXEL, 8>(leds[6], 1);

  FastLED.clear();
  FastLED.show();

  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

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

  //更新光表
  wifiMode = digitalRead(SWITCH_PIN);

  if (wifiMode) {
    Serial.println("Wi-Fi Mode");
    setupWiFiMode();
  } else {
    Serial.println("Memory Mode");
    setupMemoryMode();
  }

  Serial.println("Setup Finished OuOb");
  currentIndex = 0;

  // 啟動 UDP 接收器
  udp.begin(localPort);
  Serial.printf("UDP listening on port %d\n", localPort);
}

void loop() {
  checkHTTP();
  if (checkUDP_number() > 0) {
    ON = 1;
    startMainProgram = 1;
  }
  btn1.read();

  // 根據 API 狀態執行主程式
  if (startMainProgram) {
    // 主程式邏輯
    mainProgram();
    // testmain();
    // remoteCheck();
  }
}
