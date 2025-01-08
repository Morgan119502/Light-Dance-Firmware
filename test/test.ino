/* 主程式 */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// WiFi 設置
const char* ssid = "1805_DADA";                    // wifi名稱
const char* password = "all100pass";            // wifi密碼

// UDP 設置
WiFiUDP udp;
unsigned int localUdpPort = 12345;  // 接收的 UDP 端口
char incomingPacket[255];          // 緩衝區

// 裝置資訊
const char* device_id = "Device-001";
bool isRunning = false;

void setup() {
  delay(1000);
  Serial.begin(115200);

  // 連接 WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // 初始化 UDP
  udp.begin(localUdpPort);
  Serial.printf("Listening on UDP port %d\n", localUdpPort);
}

void loop() {
  // 檢查是否有收到 UDP 資料
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // 讀取資料
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = '\0';  // 確保是字串結尾
    }

    Serial.printf("Received packet: %s\n", incomingPacket);

    // 處理接收到的訊息
    if (strcmp(incomingPacket, "start") == 0) {
      Serial.println("Start signal received.");
      isRunning = true;
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.print(device_id);
      udp.print(": running");
      udp.endPacket();
    } else if (strcmp(incomingPacket, "stop") == 0) {
      Serial.println("Stop signal received.");
      isRunning = false;
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.print(device_id);
      udp.print(": stopped");
      udp.endPacket();
    } else if (strcmp(incomingPacket, "heartbeat") == 0) {
      Serial.println("Heartbeat signal received.");
      udp.beginPacket(udp.remoteIP(), udp.remotePort());
      udp.print(device_id);
      udp.print(": alive");
      udp.endPacket();
    }
  }

  // 模擬裝置的執行狀態
  if (isRunning) {
    Serial.println("Device is running...");
  } else {
    Serial.println("Device is stopped.");
  }

  delay(1000);  // 每秒執行一次
}
