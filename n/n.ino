/*
 * ESP-NOW 发送端（雷达端）
 * LD2450  256 k  GPIO16(RX)  5 V→3.3 V 分压
 * 每秒 10 帧  38 字节原始帧  →  ESP-NOW 广播
 * OLED: 21/47  128×64
 */

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>


#define SCREEN_W 128
#define SCREEN_H 64


#define RADAR_RX   16
#define RADAR_BAUD 256000
#define FRAME_LEN  38

HardwareSerial radar(2);
byte frame[FRAME_LEN];
int idx = 0;

uint8_t peerAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; // 广播

// 替换原来的 onSent 函数
void onSent(const uint8_t *mac, esp_now_send_status_t status) {
  if (status == ESP_NOW_SEND_SUCCESS)
    Serial.println("ESP-NOW 发送成功");
  else
    Serial.println("ESP-NOW 发送失败");
}

void setup() {
  Serial.begin(115200);
  radar.begin(RADAR_BAUD, SERIAL_8N1, RADAR_RX, 17);

  // OLED 开机动画
 

  // ESP-NOW 初始化
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (1);
  }
  esp_now_register_send_cb(onSent);
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, peerAddr, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Add peer failed");
    while (1);
  }
  Serial.println("ESP-NOW 发送端就绪");
}

void loop() {
  while (radar.available()) {
    byte b = radar.read();
    frame[idx++] = b;
    if (idx == FRAME_LEN) {
      esp_now_send(peerAddr, frame, FRAME_LEN); // 广播
      idx = 0;
    }
  }
}