#include <LittleFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <WiFiUdp.h>  // 新增此行

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SDA_PIN 12
#define SCL_PIN 13

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// IPAddress local_IP(192, 168, 230, 241);  // 替換為你的靜態 IP
// IPAddress gateway(192, 168, 1, 1);
// IPAddress subnet(255, 255, 255, 0);

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
// String deviceId = "test01";     // 裝置名稱
String deviceId = "test02";  // 裝置名稱

const int headPin = 2;
const int shoulderPin = 3;
const int chestPin = 4;
const int arm_waistPin = 5;
const int leg1Pin = 6;
const int leg2Pin = 7;
const int shoesPin = 8;

// wifi連線
void connectToWiFi() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  // WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  // 更新 OLED 顯示成功
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Connected");
  display.setCursor(0, 16);
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
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

// 處理 UDP 指令
void handleCommand(String command) {
  if (command == "start") {
    startMainProgram = true;
    running = true;
    Serial.println("Received 'start' command.");
    String response = deviceId + ": running";
    udp.beginPacket(responseAddress, responsePort);
    udp.write(response.c_str());
    udp.endPacket();
  } else if (command == "stop") {
    startMainProgram = false;
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

void mainProgram() {
  checkUDP();
  // 照著光表亮
  while (1) {
    digitalWrite(headPin, HIGH);
    checkUDP();
  }



  //
  delay(10);
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
  
  display.setCursor(0, 0);
  display.println("Start.");
  display.display();

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
}

void loop() {
  checkHTTP();
  checkUDP();

  // 根據 API 狀態執行主程式
  if (startMainProgram) {
    // 主程式邏輯
    mainProgram();
    // testmain();
    // remoteCheck();
  }
}