#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>  // 添加数学库用于计算

#define SCREEN_W 128
#define SCREEN_H 64
Adafruit_SSD1306 disp(SCREEN_W, SCREEN_H, &Wire, -1);

#define LINE_H 8
#define MAX_LINE 8
static int8_t linePtr = 0;

#define M_PI 3.14159265358979323846  // 定义圆周率

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

/* ---------- 收到 ESP-NOW 数据 ---------- */
void onRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (len != 38) return;  // 确保数据长度为 38 字节

  // 解析目标数据
  double x1 = parseCoord(data[4], data[5]);   // T1 X 米
  double y1 = parseCoord(data[6], data[7]);   // T1 Y 米
  double v1 = parseCoord(data[8], data[9]);   // T1 速度 cm/s

  double x2 = parseCoord(data[16], data[17]); // T2 X 米
  double y2 = parseCoord(data[18], data[19]); // T2 Y 米
  double v2 = parseCoord(data[20], data[21]); // T2 速度 cm/s

  double x3 = parseCoord(data[28], data[29]); // T3 X 米
  double y3 = parseCoord(data[30], data[31]); // T3 Y 米
  double v3 = parseCoord(data[32], data[33]); // T3 速度 cm/s

  // 计算距离和角度
  double distance1 = sqrt(x1 * x1 + y1 * y1);
  double angle1 = atan2(y1, x1) * (180.0 / M_PI);

  double distance2 = sqrt(x2 * x2 + y2 * y2);
  double angle2 = atan2(y2, x2) * (180.0 / M_PI);

  double distance3 = sqrt(x3 * x3 + y3 * y3);
  double angle3 = atan2(y3, x3) * (180.0 / M_PI);

  // 串口输出
  Serial.printf("T1 X:%.2f m Y:%.2f m V:%.2f cm/s Distance:%.2f m Angle:%.2f deg\n",
                x1, y1, v1, distance1, angle1);
  Serial.printf("T2 X:%.2f m Y:%.2f m V:%.2f cm/s Distance:%.2f m Angle:%.2f deg\n",
                x2, y2, v2, distance2, angle2);
  Serial.printf("T3 X:%.2f m Y:%.2f m V:%.2f cm/s Distance:%.2f m Angle:%.2f deg\n",
                x3, y3, v3, distance3, angle3);

  // OLED 显示
  oledLine("T1 X:" + String(x1, 2) + " Y:" + String(y1, 2) + " V:" + String(v1, 2) +
           " D:" + String(distance1, 2) + " A:" + String(angle1, 2));
  oledLine("T2 X:" + String(x2, 2) + " Y:" + String(y2, 2) + " V:" + String(v2, 2) +
           " D:" + String(distance2, 2) + " A:" + String(angle2, 2));
  oledLine("T3 X:" + String(x3, 2) + " Y:" + String(y3, 2) + " V:" + String(v3, 2) +
           " D:" + String(distance3, 2) + " A:" + String(angle3, 2));
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

void loop() {
  // 主循环为空，所有功能在回调函数中完成
}
