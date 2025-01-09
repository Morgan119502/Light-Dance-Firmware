#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <FastLED.h>
#include <EasyButton.h>

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
String deviceId = "test03";     // 裝置名稱

// LED腳位設定
#define switch 17
#define btnpin 16
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
unsigned int array[100][8];
EasyButton btn1(btnpin, 100, true);
bool ON = 0;
bool wifisw = 0;
int i = 0;
String memory;  //光表

// wifi連線
void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());  // 印出 IP 位址
  return;
}

void onButton() {
  ON = !ON;
  startMainProgram = !startMainProgram;
  Serial.println(ON);
  if (ON) {
    startTime = millis();
    i = 0;  //按了按鈕後是要從頭開始還是接著
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
      ON = true;
      return -2;
    }
    if (receivedNumber == 1751474546) {  // start
      handleCommand("heartbeat");
      return -2;
    }
    Serial.println(receivedNumber);
    return receivedNumber;
  }
  return -2;
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
  while (1) {
    if (checkUDP_number() == -1) break;
    btn1.read();
    if (ON) {
      i = checkUDP_number() / 1000;
      Serial.println(i);
      if (i < 100 && (millis() - startTime >= array[i][0] * 50)) {
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
}

int bright(unsigned int data) {
  return (data >> 0) & 0xFF;
}

void setup() {
  delay(1000);
  Serial.println("Start.");
  Serial.begin(115200);

  // 連接 WiFi
  connectToWiFi();

  if (!LittleFS.begin()) {
    Serial.println("Failed to initialize LittleFS");
    return;
  }

  // 啟動 HTTP 伺服器
  server.begin();

  // 啟動 UDP 接收器
  udp.begin(localPort);
  Serial.printf("UDP listening on port %d\n", localPort);

  if (tryToRcv) tryRcv();

  //fastled腳位宣告
  FastLED.addLeds<NEOPIXEL, 2>(led1, 1);
  FastLED.addLeds<NEOPIXEL, 3>(led2, 1);
  FastLED.addLeds<NEOPIXEL, 4>(led3, 1);
  FastLED.addLeds<NEOPIXEL, 5>(led4, 1);
  FastLED.addLeds<NEOPIXEL, 6>(led5, 1);
  FastLED.addLeds<NEOPIXEL, 7>(led6, 1);
  FastLED.addLeds<NEOPIXEL, 8>(led7, 1);
  pinMode(switch, INPUT_PULLUP);  //wifi mode & memory mode切換腳位
  HTTPClient http;
  //淨空
  FastLED.clear();
  FastLED.show();

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

  //連接api
  http.begin("http://140.113.160.136:8000/get_test_lightlist/cnt=300");
  JsonDocument doc;

  //更新光表
  wifisw = digitalRead(switch);
  if (wifisw) {
    Serial.println("Wifi Mode");
    for (int i = 0; i < 7; i++) {
      leds[i][0] = CRGB::Red;
      delay(500);
    }
    //處理光表輸入
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      memory = http.getString();
      Serial.println("api success");
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
  } else {
    Serial.println("Memory Mode");
    for (int i = 0; i < 7; i++) {
      leds[i][0] = CRGB::Green;
    }
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

  FastLED.show();
  delay(500);
  FastLED.clear();
  FastLED.show();
  //deserializeJson(doc, input);
  for (int i = 0; i < 100; i++) {
    array[i][0] = doc["color_data"][i]["time"];
    array[i][1] = doc["color_data"][i]["head"];
    array[i][2] = doc["color_data"][i]["shoulder"];
    array[i][3] = doc["color_data"][i]["chest"];
    array[i][4] = doc["color_data"][i]["arm_waist"];
    array[i][5] = doc["color_data"][i]["leg1"];
    array[i][6] = doc["color_data"][i]["leg2"];
    array[i][7] = doc["color_data"][i]["shoes"];
  }
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

void loop() {
  checkHTTP();
  checkUDP_number();
  btn1.read();

  // 根據 API 狀態執行主程式
  if (startMainProgram) {
    // 主程式邏輯
    mainProgram();
    // testmain();
    // remoteCheck();
  }
}
