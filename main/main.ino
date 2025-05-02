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
#include <string.h>
#include <math.h>

#define PLAYER_NUM 0 // 玩家編號

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define SDA_PIN 12
#define SCL_PIN 13

String deviceId = "player" + String(PLAYER_NUM);  // 裝置名稱

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiUDP udp;  // 建立 UDP 對象

unsigned int localPort = 12345;                 // 接收廣播的埠
char* responseAddress;
unsigned int responsePort = 12346;              // 回傳訊息的埠

// WiFi 設定
// const char* ssid = "EE219B";          // wifi名稱
// const char* ssid = "Lightdance";
char* ssid;
const char* password = "wifiyee219";  // wifi密碼

// API設定
const char* remoteUrl = "http://140.113.160.136:8000/items/eesa1/LATEST";  //最後不要加斜線!!!!
// 全域變數
WiFiServer server(80);          // 設置 HTTP 伺服器埠
bool startMainProgram = false;  // 主程式啟動開關
bool running = false;           // 模擬任務執行狀態
bool tryToRcv = true;           // 是否嘗試接收檔案
bool firstStart = true;
int offset = 0;
int num_data;

// LED腳位設定
#define DEBUG_PIN 18
#define SWITCH_PIN 17
#define BUTTON_PIN 16
#define WIFI_PIN 20
#define CNT 30 //api數量最大值
#define CHUNK_SIZE 10
#define LED_COUNT 7
#define head     0xFF3B30  // RED
#define shoulder 0xFF9500  // ORANGE
#define chest    0xFFD60A  // YELLOW
#define front    0x64DD17  // LIME GREEN
#define skirt    0x00E676  // NEON GREEN
#define leg      0x40E0D0  // AQUA
#define shoes    0x5AC8FA  // SKY BLUE
#define weap_1   0xAF52DE  // PURPLE
#define weap_2   0xFF2D55  // PINK

unsigned int part[10] = { 0, head, shoulder, chest, front, skirt, leg, shoes, weap_1, weap_2};

// LED setup
const int ledPins[LED_COUNT] = { 2, 3, 4, 5, 6, 7 };
CRGB led1[5];  //頭
CRGB led2[4];  //肩
CRGB led3[11];  //胸、手、武器
CRGB led4[4];  //腰、裙
CRGB led5[5];  //腿、鞋
CRGB led6[3];  //前
const int sectionSizes[] =    { 5, 4, 4, 5, 8, 11, 4, 4, 5, 3 };
int sectionStart[] =          { 0, 0, 0, 4, 5, 8, 0, 0, 4, 0 };
const int sectionIndices[] =  { 1, 2, 4, 5, 8, 9, 3, 6, 7, 4 };
const int sectionRows[] =     { 0, 1, 2, 2, 2, 2, 3, 4, 4, 5 };

unsigned int array[4096][10];
EasyButton btn1(BUTTON_PIN, 100, true);
CRGB* leds[6] = { led1, led2, led3, led4, led5, led6 };
unsigned long startTime = 0;
bool ON = 0;
bool wifiMode = false;

// int i = 0;
int currentIndex = 0;
String memoryData;

// Check wifi connection
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
  // display.setCursor(0, 16);
  // display.print("IP: ");
  // display.println(WiFi.localIP());
  display.display();

  return;
}

void fetchChunk(int chunk) {
  HTTPClient http;
  //String apiUrl = "http://140.113.160.136:8000/get_test_lightlist/cnt=" + String(CNT) + "/chunk=" + String(chunk);
  // String apiUrl = "http://140.113.160.136:8000/items/eesa1/2025-02-14-20:35:59";

  String apiUrl = "http://140.113.160.136:8000/items/eesa1/LATEST/player=" + String(PLAYER_NUM) + "/chunk=" + String(chunk);
  // String apiUrl = "http://140.113.160.136:8000/items/back_test/LATEST/player=0/chunk=" + String(chunk);



  http.begin(apiUrl);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    memoryData = http.getString();
    // Serial.println(apiUrl);
    // Serial.println("API fetch successful");


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
    } else {
      Serial.print("deserialization success chuck ");
      Serial.print(chunk);
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(1,1);
      display.println(chunk);
      display.display();
      Serial.println(" success");
    }
    // const char* id = doc["_id"];
    // const char* user = doc["user"];
    // const char* update_time = doc["update_time"];
    JsonArray players = doc["player_data"];

    num_data = players.size();
    Serial.print("num_data: ");
    Serial.println(num_data);

    for (int i = 0; i < num_data && i < CNT; i++) {
      JsonObject player = players[i];

      array[i + CHUNK_SIZE * chunk][0] = player["time"];
      Serial.print("time: ");
      Serial.println(array[i + CHUNK_SIZE * chunk][0]);

      array[i + CHUNK_SIZE * chunk][1] = player["head"];
      array[i + CHUNK_SIZE * chunk][2] = player["shoulder"];
      array[i + CHUNK_SIZE * chunk][3] = player["skirt"];
      array[i + CHUNK_SIZE * chunk][4] = player["chest"];
      array[i + CHUNK_SIZE * chunk][5] = player["front"]; //這是手
      array[i + CHUNK_SIZE * chunk][6] = player["leg"];
      array[i + CHUNK_SIZE * chunk][7] = player["shoes"];
      array[i + CHUNK_SIZE * chunk][8] = player["weap_1"];
      array[i + CHUNK_SIZE * chunk][9] = player["weap_2"];
    }
  } else {
    Serial.printf("API fetch failed with error code: %d\n", httpResponseCode);
  }
  http.end();
  delay(20);
}

// Setup for Wi-Fi mode
void setupWiFiMode() {
  connectToWiFi();
  //fetchChunk();
  for (int chunk = 0; chunk < CNT; chunk++) {
    fetchChunk(chunk);
  }
  saveArrayToFile();
}

// Save data to pico memory
void saveArrayToFile() {
  File file = LittleFS.open("/array_data.bin", "w");
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }

  for (int i = 0; i < 4096; ++i) {
    for (int j = 0; j < 8; ++j) {
      file.write((uint8_t*)&array[i][j], sizeof(unsigned int));
    }
  }

  file.close();
  Serial.println("Array data saved to file");
}

// Read data from pico memory
void loadArrayFromFile() {
  File file = LittleFS.open("/array_data.bin", "r");
  if (!file) {
    Serial.println("Failed to open file for reading");
    return;
  }

  for (int i = 0; i < 4096; ++i) {
    for (int j = 0; j < 8; ++j) {
      if (file.available()) {
        file.read((uint8_t*)&array[i][j], sizeof(unsigned int));
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
  // Serial.print("data: ");
  // Serial.println(data);
  // Serial.print("   data%256: ");
  // Serial.println(data%256);
  // return ((data % 256) * 255) / 100;
  return pow(1.74 , (data % 256) / 10.0);
  // return (data >> 0) & 0xFF;
}

// Setup when btn1 pressed 
void onButton() {
  ON = true;
  startMainProgram = 1;
  firstStart = 1;
  Serial.print("startmain: ");
  Serial.println(startMainProgram);

  // Serial.println(ON);
  // if (ON) {
  //   startTime = millis();
  //   currentIndex = 0;  //按了按鈕後是要從頭開始還是接著
  // }
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
    if (receivedNumber == 1751474546) {  // heartbeat
      handleCommand("heartbeat");
      return -2;
    }
    // Serial.println(receivedNumber);
    //Serial.println("received success");
    return receivedNumber;
  }
  //Serial.println("checkfail");
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

// Show the lightlist
int currentTime = 0;
void mainProgram() {  // 照著光表亮
  Serial.println("enter main");
  while (1) {
    //Serial.println("enter loop");
    // if (ON) {
    if (ON) {
      Serial.println("on");
      startTime = millis();
      while (currentIndex < 4096) {
        // Serial.print(".");
        btn1.read();
        int ii = checkUDP_number();
        // Serial.print("currenttime: ");
        // Serial.println(ii);
        if (ii == -1) {
          display.clearDisplay();
          display.setTextSize(2);
          display.setCursor(1,1);
          display.println("\n!!stop!!");
          display.display();
          FastLED.clear();
          FastLED.show();
          return;
        }
        if (ii > 0) {
          // Serial.print(ii);
          Serial.println("startshow");
          currentTime = ii / 50;
          Serial.print("currentTime: ");
          Serial.println(currentTime);
          if (firstStart) {
            firstStart = false;
            startTime = millis();
            offset = currentTime;
            while (array[currentIndex][0] < currentTime) {
              currentIndex++;
            }
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
        if (!firstStart and millis() - startTime >= (currentTime - offset) * 50) {
          Serial.print("currentIndex: ");
          Serial.print(currentIndex);
          Serial.print("  target value: ");
          Serial.println(array[currentIndex][0]);
          // Serial.println("print");
          if (currentTime >= array[currentIndex][0]) {
            // Serial.println("bling");
            for (int i = 0; i < 10; i++) {
              for (int j = sectionStart[i]; j < sectionSizes[i]; j++) {
                leds[sectionRows[i]][j] = array[currentIndex][sectionIndices[i]] >> 8;
                leds[sectionRows[i]][j].nscale8(calculateBrightness(array[currentIndex][sectionIndices[i]]));
              }
            }
            // Serial.print("head_data: ");
            // Serial.println(array[currentIndex][1]);
            // Serial.print("head_bright: ");
            // Serial.println(calculateBrightness(array[currentIndex][1]));

            // for (int j = 0; j < LED_COUNT; j++) {
            //   leds[j][0] = array[currentIndex][j + 1] >> 8;
            //   //Serial.print(array[currentIndex][j + 1]);
            //   leds[j][0].nscale8(calculateBrightness(array[currentIndex][j + 1]));
            // }
            currentIndex++;
          }
          currentTime++;
          Serial.print("now value: ");
          Serial.println(currentTime);

          FastLED.show();
        }
      }
    }
  }
  Serial.println("out main");
}

void debug() {
  for (int i = 0; i < 10; i++) {
    for (int j = sectionStart[i]; j < sectionSizes[i]; j++) {
      leds[sectionRows[i]][j] = part[sectionIndices[i]];
    }
  }
  Serial.print("BLINK");
  FastLED.setBrightness(255);
  FastLED.show();
  delay(1000);
  while (1);
}

void setup() {
  Wire.setSDA(SDA_PIN);
  Wire.setSCL(SCL_PIN);
  Wire.begin();

  FastLED.addLeds<NEOPIXEL, 2>(leds[0], 5);
  FastLED.addLeds<NEOPIXEL, 3>(leds[1], 4);
  FastLED.addLeds<NEOPIXEL, 4>(leds[2], 11);
  FastLED.addLeds<NEOPIXEL, 5>(leds[3], 4);
  FastLED.addLeds<NEOPIXEL, 6>(leds[4], 5);
  FastLED.addLeds<NEOPIXEL, 7>(leds[5], 3);


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

  pinMode(DEBUG_PIN, INPUT_PULLUP);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(WIFI_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if (!digitalRead(DEBUG_PIN)){
    debug();
  }

  if (digitalRead(WIFI_PIN)) {
    responseAddress = "192.168.0.104";
    ssid = "EE219B";
  }
  else {
    responseAddress = "192.168.50.201";
    ssid = "Lightdance";
  }

  //while (!Serial) {}
  // 連接 WiFi
  connectToWiFi();

  if (!LittleFS.begin()) {
    Serial.println("Failed to initialize LittleFS");
    return;
  }

  // 啟動 HTTP 伺服器
  server.begin();

  // if (tryToRcv) tryRcv();

  // Initialize LED

  FastLED.clear();
  FastLED.setBrightness(255);
  FastLED.show();

  btn1.begin();
  btn1.onPressed(onButton);

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
    display.println("\nWifi Mode");
    display.display();    
    setupWiFiMode();
  } else {
    Serial.println("Memory Mode");
    display.println("Memory Mode");
    display.display();
    setupMemoryMode();
  }

  Serial.println("Setup Finished OuOb");
  currentIndex = 0;

  // 啟動 UDP 接收器
  udp.begin(localPort);
  Serial.printf("UDP listening on port %d\n", localPort);
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  if (wifiMode) {
    display.println("\nWifi Mode");
    display.display(); 
  }
  else {
    display.println("Memory Md");
    display.display();
  }
  display.println("\n!!Ready!!");
  display.display();
}

void loop() {
  // checkHTTP();
  if (checkUDP_number() > 0) {
    ON = 1;
    startMainProgram = 1;
  }
  btn1.read();

  // 根據 API 狀態執行主程式
  if (startMainProgram) {
    display.clearDisplay();
    display.setTextSize(2);    
    display.setCursor(1,1);
    display.println("\n!!run!!");
    display.display();
    // 主程式邏輯
    mainProgram();
    btn1.read();
    // testmain();
    // remoteCheck();
  }
}