/*
 * ESP-NOW 接收端（OLED 显示）（128×64  I²C 21/47）
 * 38 字节帧 → 按图例符号位解析 → 米 + cm/s
 */

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_W 128
#define SCREEN_H 64
Adafruit_SSD1306 disp(SCREEN_W, SCREEN_H, &Wire, -1);

#define LINE_H 8
#define MAX_LINE 8
static int8_t linePtr = 0;

/* ---------- 工具：小端 S16 → 米 / cm ---------- */
static int16_t rawS16(byte lo, byte hi) {
  return (int16_t)(lo | (hi << 8));
}

/* 按图例符号位规则 → 米（或 cm） */
static double parseCoord(byte lo, byte hi) {
  int16_t raw = rawS16(lo, hi);
  if (raw & 0x8000) {               // 最高位 1 → 正坐标
    return (raw - 32768) / 1000.0;  // 正：raw - 2^15 → m
  } else {                          // 最高位 0 → 负坐标
    return -raw / 1000.0;           // 负：0 - raw → m
}
}

/* ---------- OLED 软件滚动 ---------- */
void oledLine(const String& s) {
  if (linePtr >= MAX_LINE) {
    for (int y = 0; y < SCREEN_H - LINE_H; y++)
      for (int x = 0; x < SCREEN_W; x++)
        disp.drawPixel(x, y, disp.getPixel(x, y + LINE_H));
    disp.fillRect(0, SCREEN_H - LINE_H, SCREEN_W, LINE_H, SSD1306_BLACK);
    linePtr = MAX_LINE - 1;
  }
  disp.setCursor(0, linePtr * LINE_H);
  disp.print(s);
  disp.display();
  linePtr++;
}

/* ---------- 收到 38 字节 ---------- */
void onRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len != 38) return;

  double x = parseCoord(data[4], data[5]);   // X 米
  double y = parseCoord(data[6], data[7]);   // Y 米
  double v = parseCoord(data[8], data[9]);   // 速度 cm/s

  /* 串口：原始 16 进制 + 解析后物理量 */
  Serial.printf("原始 X:0x%02X%02X Y:0x%02X%02X → X=%+6.2f m  Y=%+6.2f m  V=%+6.2f cm/s\n",
                data[5], data[4], data[7], data[6], x, y, v);

  /* OLED：米，2 位小数 */
  oledLine("X:" + String(x, 2) + " Y:" + String(y, 2));
}

/* ---------- Setup ---------- */
void setup() {
  Serial.begin(115200);
  Wire.begin(47, 21);
  disp.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  disp.clearDisplay();
  disp.setTextSize(1);
  disp.setTextColor(SSD1306_WHITE);
  disp.setCursor(0, 0);
  disp.print("ESP-NOW Rx");
  disp.display();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (1);
  }
  esp_now_register_recv_cb(onRecv);
  Serial.println("ESP-NOW 接收端就绪");
}

void loop() {}