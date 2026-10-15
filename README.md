# Phelan Butterfly V1 — ESP32 Flapping-Wing Flight Controller (Bản Test Code)

[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32%20%7C%20ESP32--C3-blue.svg)](#)
[![Status: Test Code](https://img.shields.io/badge/Status-Bản%20Test%20Code-orange.svg)](#)

> [!NOTE]
> **Lưu ý**: Đây là **bản test code** thử nghiệm cho dự án mô hình bướm vỗ cánh (Phelan Butterfly). Bản chính thức đầy đủ tính năng sẽ sớm ra mắt ở phần bên dưới!

> Hệ thống điều khiển mô hình bướm vỗ cánh (Ornithopter) 2 servo dùng vi điều khiển ESP32 / ESP32-C3, trang bị cặp servo kỹ thuật số kim loại **PTK 7452** và hệ thống nguồn **Pin LiPo 2S (7.4V)** qua mạch hạ áp **BEC 5V**, tích hợp giao diện mặt đất **Tactical Web Deck** qua Wi-Fi thời gian thực.

---

## Mục Lục

- [Điểm Nổi Bật](#điểm-nổi-bật)
- [Cấu Trúc Dự Án](#cấu-trúc-dự-án)
- [Cấu Hình Phần Cứng & Nguồn Điện](#cấu-hình-phần-cứng--nguồn-điện)
- [Sơ Đồ Kết Nối (Pinout & Wiring)](#sơ-đồ-kết-nối-pinout--wiring)
- [Thư Viện Phần Mềm](#thư-viện-phần-mềm)
- [Hướng Dẫn Sử Dụng](#hướng-dẫn-sử-dụng)
- [Tùy Chỉnh Thông Số](#tùy-chỉnh-thông-số)
- [Simulator](#simulator)
- [Bản Đầy Đủ (Sắp Ra Mắt)](#bản-đầy-đủ-sắp-ra-mắt)
- [Đóng Góp](#đóng-góp)
- [Giấy Phép](#giấy-phép)

---

## Điểm Nổi Bật

| Tính năng | Mô tả |
| :--- | :--- |
| **Cặp Servo PTK 7452** | Điều khiển trực tiếp 2 servo kỹ thuật số kim loại PTK 7452 ở tần số xung 300Hz, lực kéo khỏe, chịu tải lực cản cánh và chống rơ nhông khi vỗ liên tục |
| **Hệ Thống Nguồn 2S + BEC** | Sử dụng pin LiPo 2S (7.4V - 8.4V) phối hợp mạch hạ áp BEC 5V (3A - 5A) độc lập, cung cấp dòng xả tức thời đủ lớn cho 2 servo mà không gây sụt áp vi điều khiển |
| **Plug & Play** | Mở trình duyệt bất kỳ (Chrome / Safari trên iOS & Android) — không cần cài App |
| **SoftAP Wi-Fi** | Cấp nguồn pin là tự phát Wi-Fi riêng, không cần router |
| **Anti-Brownout** | Vô hiệu hóa bộ phát hiện sụt áp sớm (`disable_bod_early`), kết hợp cấu hình nguồn BEC giải quyết triệt để lỗi reset ESP32 khi 2 servo ăn dòng đỉnh |
| **Throttle Interlock** | Bắt buộc hạ cần ga về 0% mới cho phép ARM, chống quạt gãy sườn hoặc gãy cánh ngoài ý muốn |
| **Failsafe 500ms** | Mất kết nối Wi-Fi/Web quá 500ms — tự động ngắt ga và đưa servo về góc lượn an toàn (`FAILSAFE_DIHEDRAL`) |
| **Emergency STOP** | Chạm 1 nút dừng toàn bộ hệ thống ngay lập tức |
| **Throttle Hold** | Cần ga giữ nguyên vị trí, cần lái hướng (Yaw, Pitch, Roll) tự động hồi tâm khi nhả tay |

---

## Cấu Trúc Dự Án

```
buomv1/
├── buomv1.ino                                       Khởi tạo & vòng lặp chính (Setup / Loop orchestrator)
├── config.h                                         Cấu hình phần cứng, GPIO, tần số PWM, Wi-Fi, gói tin
├── servo_sys.h / .cpp                               Điều khiển servo PTK 7452, lọc EMA, giới hạn an toàn
├── flight.h / .cpp                                  Khí động học cánh vỗ, bảng LUT, bù biên độ, FreeRTOS task
├── web_server.h / .cpp                              Wi-Fi STA/SoftAP, mDNS, WebSocket server, Telemetry
├── web_ui.h                                         Giao diện Web Deck nhúng PROGMEM HTML/CSS/JS
├── index.html                                       File giao diện gốc phục vụ phát triển & test offline
├── server.js                                        Simulator WebSocket trên PC (dev/test không cần ESP32)
├── Dimension_A1_print_Phelan_butterfly_ 80cm.pdf    Bản vẽ thiết kế & kích thước khung cánh 80cm
├── .gitignore
├── LICENSE
└── README.md
```

---

## Cấu Hình Phần Cứng & Nguồn Điện

### 1. Danh Sách Linh Kiện

| Thành phần | Quy cách khuyến nghị | Ghi chú kỹ thuật |
| :--- | :--- | :--- |
| **Vi điều khiển** | ESP32 Dev Module hoặc ESP32-C3 Dev Module | Xung nhịp 160MHz - 240MHz, tích hợp Wi-Fi + mDNS |
| **Servo (x2)** | **PTK 7452** (hoặc PTK 7452 MG-D) | Digital Metal Gear, xung điều khiển 300Hz, điện áp 4.8V - 6.0V |
| **Nguồn pin** | **LiPo 2S 7.4V** (300mAh – 650mAh, 30C - 75C) | Dòng xả cao, trọng lượng nhẹ, đảm bảo công suất vỗ cánh |
| **Mạch hạ áp (BEC)** | **BEC rời 5V / 3A – 5A** (hoặc Step-Down UBEC) | Bắt buộc: hạ áp từ 2S (7.4V) xuống 5V ổn định nuôi cặp servo PTK 7452 |
| **Khung cánh** | Khung bướm Phelan sải cánh 80cm | Kèm bản vẽ in khổ A1 (`Dimension_A1_print_Phelan_butterfly_ 80cm.pdf`) |

### 2. Nguyên Lý Cấp Nguồn An Toàn (Power Architecture)

Để đảm bảo ESP32 không bị sập nguồn khi 2 servo PTK 7452 đồng thời đổi chiều ở tần số 300Hz:
1. **Nguồn Servo**: Chân `VCC (+)` của 2 servo PTK 7452 lấy trực tiếp từ đầu ra **5V của mạch BEC**. Tuyệt đối **không** lấy nguồn servo từ chân `5V/VIN` của kit ESP32.
2. **Nguồn ESP32**: Đầu ra 5V của BEC đồng thời cấp vào chân `VIN` (hoặc `5V`) của ESP32.
3. **Chung Mass (Common GND)**: Nối chung toàn bộ cực âm `GND` của Pin 2S, BEC, Kit ESP32 và 2 servo PTK 7452.

---

## Sơ Đồ Kết Nối (Pinout & Wiring)

> [!CAUTION]
> **CẢNH BÁO QUAN TRỌNG VỀ PINOUT (ESP32 WROOM / ESP32 Dev Module):**
> Tuyệt đối **KHÔNG sử dụng GPIO 6 đến GPIO 11** trên kit ESP32 tiêu chuẩn. Các chân này được nối trực tiếp với chip nhớ SPI Flash nội bộ. Nếu gán GPIO 6 hoặc 8 làm chân servo/LED, ESP32 sẽ bị xung đột bus bộ nhớ Flash và rơi vào vòng lặp Bootloop/Watchdog Reset: `rst:0x8 (TG1WDT_SYS_RESET)`.

### 1. Kit Chuẩn ESP32 DevKit V1 (DOIT ESP32 DEVKIT V1 / ESP-WROOM-32 — Khuyến Nghị Test)

> Phiên bản code mặc định được tối ưu và kiểm thử trực tiếp trên dòng kit **DOIT ESP32 DEVKIT V1** (30 chân hoặc 36 chân).

| Linh Kiện | Chân Tín Hiệu Trên Kit | Chân Nguồn | Chức Năng |
| :--- | :---: | :---: | :--- |
| **Servo PTK 7452 Trái (L)** | `D21` (GPIO 21) | BEC 5V / GND | Động lực vỗ cánh + lái vi sai cánh trái (PWM 300Hz) |
| **Servo PTK 7452 Phải (R)** | `D22` (GPIO 22) | BEC 5V / GND | Động lực vỗ cánh + lái vi sai cánh phải (PWM 300Hz) |
| **LED Trạng Thái** | `D2` (GPIO 2) | Onboard / GND | Đèn LED xanh dương tích hợp trên kit DevKit V1 |
| **Nguồn Cấp Cho Kit** | Chân `VIN` | BEC 5V (+) | Cấp nguồn 5V ổn định từ đầu ra BEC |
| **Mass Toàn Mạch** | Chân `GND` | Common GND (-) | Nối chung toàn bộ cực âm pin, BEC và servo |

### 2. Kit Nhỏ Gọn ESP32-C3 Dev Module

| Linh Kiện | Chân Tín Hiệu | Chân Nguồn | Chức Năng |
| :--- | :---: | :---: | :--- |
| **Servo PTK 7452 Trái (L)** | `GPIO 4` | BEC 5V / GND | Động lực vỗ cánh + lái vi sai cánh trái (PWM 300Hz) |
| **Servo PTK 7452 Phải (R)** | `GPIO 5` | BEC 5V / GND | Động lực vỗ cánh + lái vi sai cánh phải (PWM 300Hz) |
| **LED Trạng Thái** | `GPIO 8` | Onboard / GND | LED trạng thái (RGB/IO8 trên kit ESP32-C3) |
| **Nguồn Pin 2S (7.4V)** | — | Vào BEC IN (+/-) | Cung cấp toàn bộ công suất hoạt động cho hệ thống |

---

## Thư Viện Phần Mềm

Cài đặt thông qua Library Manager trên Arduino IDE:

1. [ESP32Servo](https://github.com/madhephaestus/ESP32Servo) — Kevin Harrington, John K. Bennett
2. [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) + [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)

Board package yêu cầu: **ESP32 by Espressif** (>= 2.0.0) trong Boards Manager.

---

## Hướng Dẫn Sử Dụng

### 1. Nạp Firmware Cho ESP32 DevKit V1

1. Mở file [buomv1.ino](file:///d:/buomv1/buomv1.ino) bằng Arduino IDE 2.x.
2. Cấu hình menu **Tools**:
   - **Board**: `DOIT ESP32 DEVKIT V1` (hoặc `ESP32 Dev Module`).
   - **Upload Speed**: `921600` (hoặc `115200` nếu cáp kết nối chập chờn).
   - **CPU Frequency**: `240MHz (WiFi/BT)` hoặc `160MHz`.
   - **Flash Frequency**: `80MHz`.
   - **Partition Scheme**: `Default 4MB with spiffs` hoặc `Huge APP (3MB No OTA)`.
   - **Port**: Chọn đúng cổng COM của chip nạp (CP2102 / CH340).
3. Cấu hình thông tin Wi-Fi trong [config.h](file:///d:/buomv1/config.h) (để trống `WIFI_STA_SSID` để luôn chạy mạng SoftAP độc lập `Phelan-Butterfly`).
4. Bấm **Upload**. *(Nếu Arduino IDE dừng ở thông báo `Connecting........_____`, hãy nhấn giữ nút **BOOT** trên kit DevKit V1 1-2 giây rồi thả ra để vào chế độ nạp flash).*

### 2. Kết Nối Thiết Bị

1. Cắm giắc pin LiPo 2S vào mạch BEC. Đèn LED trên vi điều khiển nhấp nháy báo hiệu khởi động.
2. Khi hệ thống sẵn sàng, kết nối điện thoại vào mạng Wi-Fi do bướm phát hoặc mạng chung.
3. Mở Serial Monitor (baud rate 115200) để kiểm tra IP hoặc dùng mDNS.

### 3. Điều Khiển Bay

1. Mở trình duyệt web (Safari trên iOS hoặc Chrome trên Android), truy cập:
   * **`http://phelan.local`** (hoặc địa chỉ IP hiển thị trên Serial Monitor).
2. Xoay ngang màn hình thiết bị để kích hoạt giao diện Tactical Flight Deck.
3. Kéo cần ga về vị trí `0%`, sau đó bấm nút **ARM** để kích hoạt lực điều khiển servo.
4. Thao tác điều khiển:
   * **Cần Trái**: Đẩy lên để tăng ga vỗ cánh (Throttle), gạt trái/phải để chuyển hướng (Yaw).
   * **Cần Phải**: Gạt lên/xuống để Chúi/Ngửa (Pitch), gạt trái/phải để Nghiêng (Roll).
   * **Nút STOP**: Ngắt toàn bộ hoạt động khẩn cấp khi gặp sự cố.

---

## Tùy Chỉnh Thông Số

Mở [config.h](file:///d:/buomv1/config.h), hiệu chỉnh tại namespace `Config`:

| Tham số | Giá trị chuẩn PTK 7452 | Đơn vị | Ý nghĩa kỹ thuật |
| :--- | :---: | :---: | :--- |
| `SERVO_HZ` | `300` | Hz | Tần số PWM điều khiển servo kỹ thuật số PTK 7452 |
| `SERVO_CENTER` | `90` | deg | Vị trí góc trung lập cơ khí của servo |
| `SERVO_MIN_ANGLE` | `20` | deg | Giới hạn góc quét cơ học dưới |
| `SERVO_MAX_ANGLE` | `160` | deg | Giới hạn góc quét cơ học trên |
| `FLAP_AMP` | `65` | deg | Biên độ góc quét vỗ cánh cực đại khi kéo ga tối đa |
| `FLAP_SPEED_MIN` | `157286` | — | Bước tăng chu kỳ vỗ cánh ở mức ga thấp nhất |
| `FLAP_SPEED_MAX` | `262144` | — | Bước tăng chu kỳ vỗ cánh ở mức ga cao nhất |
| `WS_FAILSAFE_MS` | `500` | ms | Thời gian timeout mất kết nối Wi-Fi/Web trước khi kích hoạt Failsafe |
| `FAILSAFE_DIHEDRAL` | `15` | deg | Góc nâng cánh chữ V tự lượn khi mất tín hiệu điều khiển |
| `REVERSE_SERVO_L` | `false` | bool | Đảo chiều servo trái nếu lắp ngược càng truyền động |
| `REVERSE_SERVO_R` | `false` | bool | Đảo chiều servo phải nếu lắp ngược càng truyền động |

---

## Simulator

Dự án cung cấp file [server.js](file:///d:/buomv1/server.js) để giả lập trạm phát ESP32 trên máy tính, phục vụ kiểm thử giao diện Tactical Web Deck mà không cần cấp nguồn phần cứng thật:

```bash
node server.js
```
Truy cập qua trình duyệt tại: `http://localhost:8080`

Yêu cầu môi trường Node.js >= 16. Server mô phỏng đầy đủ giao thức nhị phân WebSocket, truyền nhận telemetry và phản hồi gói tin Heartbeat PING/PONG.

---

## Bản Đầy Đủ (Sắp Ra Mắt)

> [!IMPORTANT]
> **Phiên bản hoàn chỉnh & đầy đủ tính năng sẽ sớm ra mắt!**
>
> Bản hiện tại là **bản test code** phục vụ việc thử nghiệm mô hình cánh vỗ Butterfly, tinh chỉnh hành trình servo và kiểm tra khả năng điều khiển qua giao diện Web Deck. Phiên bản đầy đủ sắp tới sẽ được công bố với các nội dung chi tiết:
> - Hướng dẫn gia công cơ khí & lắp ráp chi tiết theo bản vẽ khung sải cánh 80cm.
> - Thuật toán ổn định khí động học và hỗ trợ tự động giữ độ cao/hướng bay.
> - Hỗ trợ tay cầm điều khiển vật lý (GamePad Bluetooth, tay điều khiển RC chuyên dụng).
> - Giao diện Web GCS nâng cao tích hợp đồ thị phân tích trạng thái thời gian thực.
>
> 🚀 *Theo dõi kho lưu trữ (Star & Watch repo) để nhận thông báo sớm nhất ngay khi bản đầy đủ chính thức được phát hành!*

---

## Đóng Góp

1. Fork kho lưu trữ này.
2. Tạo nhánh tính năng: `git checkout -b feature/ten-tinh-nang`.
3. Lưu commit: `git commit -m "feat: mo ta ngan ve tinh nang"`.
4. Đẩy nhánh: `git push origin feature/ten-tinh-nang`.
5. Tạo Pull Request để xem xét tích hợp.

Mọi vấn đề phát sinh hoặc đề xuất cải tiến xin vui lòng gửi qua mục [Issues](../../issues).

---

## Giấy Phép

Phát hành theo giấy phép [MIT License](LICENSE) — Bản quyền (c) 2026 PhelanDev.
