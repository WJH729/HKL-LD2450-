🎯 LD2450-ESPNOW-Display
无路由器、零配置的 LD2450 雷达实时坐标系统 —— 用 ESP-NOW 把雷达数据 毫秒级 推送到 OLED + Web 网页，2 m 墙测误差 < 5 cm！
✨ 特性一览
表格
复制
模块	实现
雷达→ESP-NOW	38 字节原帧广播，无需 Wi-Fi / 蓝牙
接收端	128×64 OLED + WebSocket 网页 双实时显示
网页坐标	2 m × 2 m 可缩放坐标轴，红点毫秒移动
精度	按 LD2450 官方符号位规则 解析，误差 < 5 cm
延迟	< 10 ms（ESP-NOW 硬件广播）
📦 开箱即用
克隆/下载本仓库
Arduino IDE 安装 依赖库
修改 Wi-Fi 名/密码 → 上传 接收端
上传发射端 → 上电 → 打开网页 → 实时红点！
🔧 硬件清单
表格
复制
设备	数量	备注
ESP32 任意型号	2	一块 发射，一块 接收
LD2450 雷达	1	5 V 供电，TX→分压→GPIO16
128×64 OLED	1	I²C 接口（SSD1306）
分压电阻	2	1 kΩ + 2.2 kΩ（5 V→3.3 V）
杜邦线	若干	共地必须
🔌 引脚对照
表格
复制
功能	雷达端	接收端
OLED SCL	—	GPIO 21
OLED SDA	—	GPIO 47
雷达 RX	GPIO 16	—
供电	5 V（LD2450）	3.3 V（ESP32）
共地	必须连接	必须连接
⚠️ 雷达 TX 必须分压到 3.3 V，否则烧毁 GPIO！
📚 依赖库
Arduino IDE → 工具 → 管理库 搜索并安装：
Adafruit GFX Library
Adafruit SSD1306
WebSockets（作者 Links2004）
ESP32 板支持包 ≥ 2.0.0（自带 WiFi/WebServer）
🚀 快速开始
① 发射端（雷达）
打开 Tx_LD2450_ESP_NOW.ino
修改分压引脚（若换脚）
上传 → 串口出现：
复制
ESP-NOW 发送端就绪
ESP-NOW 发送成功
② 接收端（显示）
打开 Rx_LD2450_ESP_NOW_Web.ino
修改 Wi-Fi 名/密码：
cpp
复制
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASS";
上传 → 串口出现：
复制
IP:192.168.x.xxx
Web 服务器就绪  http://192.168.x.xxx
③ 打开网页
浏览器输入 http://192.168.x.xxx → 红点实时移动！
📊 精度验证
表格
复制
场景	网页读数	实际距离	误差
墙 1 m	1.00 m	1.00 m	0 cm
墙 2 m	2.01 m	2.00 m	1 cm
人手 50 cm	0.49 m	0.50 m	1 cm
屏住呼吸 后误差 < 5 cm！
🌐 网页坐标轴
2 m × 2 m 默认范围（可改 scale 变量）
红点 = 目标位置
实时刷新 = WebSocket 推送（≈10 ms）
🛠️ 自定义
表格
复制
功能	变量
坐标范围	const scale = 100;（100 px = 1 m）
OLED 字体	disp.setTextSize(1);
推送间隔	由雷达 10 Hz 决定
📄 协议说明
38 字节帧 → 符号位解析（官方图例）：
正坐标：raw - 32768 → 米
负坐标：0 - raw → 米
单位：mm → m（直接除 1000）
🤝 贡献 & 反馈
欢迎 Issue / PR 提交：
精度优化
更多显示方式
其他雷达型号适配
📜 许可证
MIT License - 可商用、可修改、可分发，请注明出处。
👉 现在就上传，打开网页 → 红点跟着你动！
