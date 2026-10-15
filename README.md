# Phelan Butterfly V1 — ESP32 Flapping-Wing Flight Controller (Bản Test Code)

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32%20DevKit%20V1-blue.svg)](#)
[![Status: Test Code](https://img.shields.io/badge/Status-Bản%20Test%20Code-orange.svg)](#)

> [!NOTE]
> **Lưu ý**: Đây là **bản test code thử nghiệm** tập trung vào điều khiển góc quét servo, thuật toán vỗ cánh và giao diện mặt đất Web Deck cho mô hình bướm vỗ cánh (Phelan Butterfly). Bản chính thức đầy đủ tính năng kèm bản vẽ và hướng dẫn chế tạo chi tiết sẽ sớm ra mắt ở phần bên dưới!

Hệ thống điều khiển mô hình bướm vỗ cánh (Ornithopter) 2 servo sử dụng vi điều khiển **ESP32 DevKit V1**, trang bị cặp servo kỹ thuật số kim loại **PTK 7452**, nguồn pin **LiPo 2S (7.4V)** qua mạch hạ áp **BEC 5V**, tích hợp giao diện điều khiển mặt đất **Tactical Web Deck** qua Wi-Fi thời gian thực.

---

## Mục Lục

- [Điểm Nổi Bật](#điểm-nổi-bật)
- [Cấu Trúc Thư Mục](#cấu-trúc-thư-mục)
- [Cấu Hình Phần Cứng & Nguồn Điện](#cấu-hình-phần-cứng--nguồn-điện)
- [Sơ Đồ Kết Nối (Pinout & Wiring)](#sơ-đồ-kết-nối-pinout--wiring)
- [Thư Viện Phần Mềm Cần Thiết](#thư-viện-phần-mềm-cần-thiết)
- [Hướng Dẫn Nạp & Vận Hành](#hướng-dẫn-nạp--vận-hành)
- [Giao Diện Điều Khiển Tactical Web Deck](#giao-diện-điều-khiển-tactical-web-deck)
- [Cơ Chế Hoạt Động & Chế Độ Đậu (Perch)](#cơ-chế-hoạt-động--chế-độ-đậu-perch)
- [Tùy Chỉnh Thông Số Trong Code](#tùy-chỉnh-thông-số-trong-code)
- [Simulator Giả Lập Trên PC](#simulator-giả-lập-trên-pc)
- [Bản Đầy Đủ (Sắp Ra Mắt)](#bản-đầy-đủ-sắp-ra-mắt)
- [Giấy Phép](#giấy-phép)

---

## Điểm Nổi Bật

| Tính năng | Mô tả chi tiết |
| :--- | :--- |
| **Cặp Servo PTK 7452** | Điều khiển trực tiếp 2 servo kỹ thuật số bánh răng kim loại PTK 7452 ở tần số xung 300Hz, lực kéo khỏe, chịu tải lực cản không khí và chống rơ nhông khi vỗ liên tục |
| **Tối Ưu ESP32 DevKit V1** | Cấu hình mặc định cho dòng kit DOIT ESP32 DevKit V1 (30 chân / 36 chân), cấp phát trọn vẹn 4 timer phần cứng cho PWM servo |
| **Hệ Thống Nguồn 2S + BEC** | Pin LiPo 2S (7.4V - 8.4V) cấp qua mạch hạ áp rời BEC 5V (3A - 5A), cung cấp dòng đỉnh tức thời ổn định cho 2 servo mà không gây sụt áp vi điều khiển |
| **Web GCS Không Cần Cài App** | Điều khiển trực tiếp trên trình duyệt (Safari iOS, Chrome Android) với giao diện buồng lái Tactical Web Deck xoay ngang màn hình |
| **Wi-Fi SoftAP Tự Phát** | ESP32 tự phát mạng Wi-Fi riêng `Phelan-Butterfly` (IP cố định `192.168.4.1` hoặc tên miền mDNS `http://phelan.local`) |
| **Chế Độ Đậu Thông Minh (Perch)** | Hai cánh gập ngược chữ V lên cao (30° - 55°) để đậu trên cành hoa/mặt đất, tự động thoát đậu khi đẩy ga bay |
| **Throttle Interlock Bảo Vệ** | Bắt buộc kéo cần ga về 0% mới cho phép nhấn ARM, ngăn ngừa vỗ cánh bất ngờ gây hỏng cơ cấu truyền động |
| **Failsafe Tự Động 500ms** | Mất kết nối Wi-Fi quá 500ms, hệ thống tự động cắt ga và đưa 2 cánh về góc lượn an toàn 15° |
| **Nút Dừng Khẩn Cấp STOP** | Một chạm dừng tức thì toàn bộ servo và ngắt trạng thái bay |

---

## Cấu Trúc Thư Mục

```
buomv1/
├── buomv1.ino          Điểm khởi đầu chương trình (Setup & Loop chính)
├── config.h            Cấu hình phần cứng GPIO, tần số PWM, tham số bay, mạng Wi-Fi
├── servo_sys.h / .cpp  Hệ thống quản lý servo, lọc EMA mượt chuyển động, đảo chiều
├── flight.h / .cpp     Bộ điều phối khí động học cánh vỗ, bảng LUT, FreeRTOS Core 1 Task
├── web_server.h / .cpp Máy chủ Wi-Fi AP/STA, mDNS, WebSocket server và truyền Telemetry
├── web_ui.h            Giao diện Web Deck nhúng bộ nhớ PROGMEM Flash
├── index.html          Giao diện gốc HTML5/CSS3/JavaScript phục vụ phát triển & test offline
├── server.js           Mô phỏng máy chủ WebSocket giả lập trên PC (test không cần ESP32)
├── .gitignore          Danh sách loại trừ tệp nhị phân và tệp rác
├── LICENSE             Giấy phép mã nguồn mở MIT
└── README.md           Tài liệu hướng dẫn chi tiết dự án
```

---

## Cấu Hình Phần Cứng & Nguồn Điện

### 1. Danh Sách Linh Kiện Khuyến Nghị

| Thành phần | Quy cách khuyến nghị | Ghi chú |
| :--- | :--- | :--- |
| **Vi điều khiển** | DOIT ESP32 DEVKIT V1 (hoặc ESP-WROOM-32) | CPU 2 nhân 240MHz, tích hợp Wi-Fi + Bluetooth |
| **Servo (x2)** | PTK 7452 hoặc PTK 7452 MG-D | Digital Metal Gear, PWM 300Hz, lực kéo 2.2kg/cm |
| **Pin cấp nguồn** | LiPo 2S 7.4V (300mAh - 650mAh, xả 30C - 75C) | Dòng xả cao, trọng lượng nhẹ phù hợp mô hình bay |
| **Mạch hạ áp (BEC)** | UBEC rời 5V / 3A - 5A | Hạ áp từ 7.4V xuống 5V ổn định nuôi cặp servo |

### 2. Nguyên Lý Đấu Nối Nguồn An Toàn

Để tránh hiện tượng vi điều khiển bị khởi động lại (Brownout Reset) khi 2 servo đồng thời đổi chiều ở tần số cao:
1. **Nguồn cấp cho servo**: Dây đỏ `VCC (+)` của 2 servo nối trực tiếp vào đầu ra **5V của mạch BEC**. Tuyệt đối không lấy nguồn cho servo từ chân `5V/VIN` của kit ESP32.
2. **Nguồn cấp cho ESP32**: Đầu ra 5V của mạch BEC đồng thời nối vào chân `VIN` của kit ESP32 DevKit V1.
3. **Nối chung Mass (Common GND)**: Bắt buộc hàn nối chung toàn bộ cực âm `GND` của Pin LiPo 2S, mạch BEC, kit ESP32 và 2 servo.

---

## Sơ Đồ Kết Nối (Pinout & Wiring)

### Kit Chuẩn ESP32 DevKit V1 (ESP-WROOM-32)

| Thiết bị | Chân trên Kit ESP32 DevKit V1 | Nguồn cấp | Chức năng |
| :--- | :---: | :---: | :--- |
| **Servo Trái (L)** | `D21` (GPIO 21) | BEC 5V / Common GND | Dây tín hiệu PWM cánh trái (tần số 300Hz) |
| **Servo Phải (R)** | `D22` (GPIO 22) | BEC 5V / Common GND | Dây tín hiệu PWM cánh phải (tần số 300Hz) |
| **LED Trạng Thái** | `D2` (GPIO 2) | Tích hợp trên board | LED báo hiệu: chớp chậm (chờ kết nối), chớp nhanh (DISARMED), sáng liên tục (ARMED) |
| **Nguồn Kit ESP32** | Chân `VIN` | BEC 5V (+) | Nhận nguồn nuôi 5V từ mạch BEC |
| **Mass Toàn Mạch** | Chân `GND` | Common GND (-) | Cực âm chung toàn hệ thống |

> [!CAUTION]
> Tuyệt đối không nối chân servo vào nhóm chân `GPIO 6 đến GPIO 11` trên ESP32 vì đây là bus giao tiếp trực tiếp với bộ nhớ Flash nội bộ, nối vào sẽ gây lỗi treo kit liên tục (`rst:0x8 TG1WDT_SYS_RESET`).

---

## Thư Viện Phần Mềm Cần Thiết

Cài đặt thông qua Library Manager trên Arduino IDE:

1. **ESP32Servo** (bởi Kevin Harrington, John K. Bennett) — Quản lý PWM điều khiển servo trên ESP32.
2. **ESPAsyncWebServer** (bởi me-no-dev) — Máy chủ web bất đồng bộ tốc độ cao.
3. **AsyncTCP** (bởi me-no-dev) — Nền tảng socket TCP bất đồng bộ cho ESP32.

Yêu cầu phiên bản Board Manager: **ESP32 by Espressif** (>= 2.0.0).

---

## Hướng Dẫn Nạp & Vận Hành

### 1. Nạp Firmware Bằng Arduino IDE

1. Mở file [buomv1.ino](file:///d:/buomv1/buomv1.ino) trên Arduino IDE 2.x.
2. Cấu hình menu **Tools**:
   - **Board**: `DOIT ESP32 DEVKIT V1` (hoặc `ESP32 Dev Module`).
   - **Upload Speed**: `921600` (hoặc `115200` nếu dây cáp tiếp xúc kém).
   - **CPU Frequency**: `240MHz (WiFi/BT)`.
   - **Flash Frequency**: `80MHz`.
   - **Partition Scheme**: `Default 4MB with spiffs` hoặc `Huge APP (3MB No OTA)`.
   - **Port**: Chọn đúng cổng COM nạp.
3. Cấu hình mạng trong [config.h](file:///d:/buomv1/config.h):
   - Mặc định hệ thống tự phát mạng riêng SoftAP: SSID `Phelan-Butterfly` (không đặt mật khẩu để kết nối ngay).
4. Nhấn nút **Upload** trên Arduino IDE. Nếu IDE hiện thông báo `Connecting........_____`, hãy nhấn giữ nút **BOOT** trên kit DevKit V1 khoảng 1-2 giây rồi thả ra.

### 2. Kết Nối & Điều Khiển

1. Cấp nguồn pin cho bướm. Đèn LED trên kit sẽ nhấp nháy báo hiệu đang phát mạng.
2. Trên điện thoại, kết nối vào mạng Wi-Fi **`Phelan-Butterfly`**.
3. Mở trình duyệt web truy cập địa chỉ:
   - **`http://192.168.4.1`** hoặc **`http://phelan.local`**
4. Xoay ngang màn hình điện thoại để vào giao diện Tactical Flight Deck.

---

## Giao Diện Điều Khiển Tactical Web Deck

* **Cần Trái (Left Stick)**:
  - Trục dọc: Điều chỉnh ga vỗ cánh (**Throttle 0% - 100%**). Cần ga có tính năng giữ vị trí (Throttle Hold).
  - Trục ngang: Chuyển hướng cánh (**Yaw -100% đến +100%**). Tự động hồi tâm khi nhả tay.
* **Cần Phải (Right Stick)**:
  - Trục dọc: Góc chúi/ngửa cánh (**Pitch -100% đến +100%**).
  - Trục ngang: Góc nghiêng vi sai cánh (**Roll -100% đến +100%**). Tự động hồi tâm khi nhả tay.
* **Nút ARM**: Kích hoạt cấp xung điều khiển cho servo. Bắt buộc cần ga ở 0% mới có thể ARM.
* **Nút PERCH**: Bật/tắt chế độ đậu gập cánh.
* **Nút STOP**: Ngắt toàn bộ hoạt động khẩn cấp, đưa servo về trạng thái an toàn.
* **Màn hình HUD**: Hiển thị thời gian chạy (UPTIME), độ trễ mạng (PING RTT tính bằng ms), nhiệt độ vi xử lý và trạng thái kết nối.

---

## Cơ Chế Hoạt Động & Chế Độ Đậu (Perch)

* **Trạng thái bay bình thường**:
  - Khi cần ga > 0%, cánh vỗ theo dạng sóng tam giác tối ưu khí động lực học dựa trên bảng tra LUT.
  - Cần lái Yaw và Roll được cộng hưởng điều khiển góc vi sai giữa hai cánh để rẽ hướng trái/phải.
* **Chế độ đậu (Perch Mode)**:
  - Kích hoạt khi nhấn nút **PERCH** trên giao diện lúc cần ga ở 0% (hoặc kéo cần Pitch xuống dưới ngưỡng kích hoạt).
  - Hai cánh sẽ đồng loạt nâng gập lên cao tạo thành góc chữ V (mặc định 30° và có thể kéo tối đa 55° bằng cần Pitch).
  - **Tự động thoát đậu**: Ngay khi phi công đẩy cần ga lên để bay, hệ thống tự động thoát chế độ đậu, hai cánh hạ về vị trí làm việc và bắt đầu vỗ cánh.

---

## Tùy Chỉnh Thông Số Trong Code

Toàn bộ thông số vận hành được quản lý tập trung trong file [config.h](file:///d:/buomv1/config.h):

| Hằng số | Giá trị chuẩn | Ý nghĩa kỹ thuật |
| :--- | :---: | :--- |
| `SERVO_HZ` | `300` | Tần số điều khiển PWM (Hz) cho servo kỹ thuật số PTK 7452 |
| `SERVO_CENTER` | `90` | Góc trung lập cơ khí của servo (độ) |
| `SERVO_MIN_ANGLE` | `20` | Giới hạn góc quét cơ học dưới an toàn (độ) |
| `SERVO_MAX_ANGLE` | `160` | Giới hạn góc quét cơ học trên an toàn (độ) |
| `FLAP_AMP` | `65` | Biên độ góc quét vỗ cánh cực đại khi kéo ga tối đa (độ) |
| `PERCH_BASE_UP_ANGLE` | `30` | Góc gập cánh cơ bản khi ở chế độ đậu (độ) |
| `FAILSAFE_DIHEDRAL` | `15` | Góc nâng cánh chữ V tự lượn khi mất kết nối Wi-Fi (độ) |
| `WS_FAILSAFE_MS` | `500` | Thời gian mất tín hiệu (ms) trước khi kích hoạt Failsafe |
| `REVERSE_SERVO_L` | `false` | Đảo chiều servo trái nếu càng truyền động quay ngược |
| `REVERSE_SERVO_R` | `false` | Đảo chiều servo phải nếu càng truyền động quay ngược |

---

## Simulator Giả Lập Trên PC

Dự án tích hợp sẵn máy chủ giả lập [server.js](file:///d:/buomv1/server.js) bằng Node.js giúp kiểm thử toàn bộ giao diện Web Deck mà không cần kết nối phần cứng ESP32 thật:

```bash
node server.js
```

Sau đó mở trình duyệt truy cập: `http://localhost:8080`

Simulator mô phỏng đầy đủ giao thức nhị phân WebSocket, truyền nhận dữ liệu telemetry hai chiều, phản hồi gói tin Heartbeat PING/PONG và trạng thái gập cánh khi đậu.

---

## Bản Đầy Đủ (Sắp Ra Mắt)

> [!IMPORTANT]
> **Phiên bản hoàn chỉnh & đầy đủ tính năng sẽ sớm ra mắt!**
>
> Bản hiện tại là **bản test code** phục vụ việc thử nghiệm mô hình cánh vỗ Butterfly, tinh chỉnh hành trình servo và kiểm tra khả năng điều khiển qua giao diện Web Deck. Phiên bản đầy đủ sắp tới sẽ được công bố với các nội dung chi tiết:
> - Hướng dẫn gia công cơ khí & lắp ráp chi tiết theo bản vẽ khung sải cánh 80cm hoàn chỉnh.
> - Thuật toán ổn định khí động học và hỗ trợ tự động giữ độ cao/hướng bay.
> - Hỗ trợ tay cầm điều khiển vật lý (GamePad Bluetooth, tay điều khiển RC chuyên dụng).
> - Giao diện Web GCS nâng cao tích hợp đồ thị phân tích trạng thái thời gian thực.
>
> 🚀 *Theo dõi kho lưu trữ (Star & Watch repo) để nhận thông báo sớm nhất ngay khi bản đầy đủ chính thức được phát hành!*

---

## Giấy Phép

Phát hành theo giấy phép [MIT License](LICENSE) — Bản quyền (c) 2026 PhelanDev.
