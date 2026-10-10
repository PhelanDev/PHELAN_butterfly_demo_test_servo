#pragma once

#include "esp_wifi.h"
#include "hal/brownout_ll.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#include <Arduino.h>
#include <atomic>

inline void disableBrownout() {
#if defined(RTC_CNTL_BROWN_OUT_REG)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
#endif
}

namespace Config {
#if defined(CONFIG_IDF_TARGET_ESP32C3)
constexpr uint8_t SERVO_L_PIN = 4;
constexpr uint8_t SERVO_R_PIN = 5;
constexpr uint8_t LED_PIN = 8;
#else
// Standard ESP32 (ESP-WROOM-32 / ESP32 Dev Module)
// CẢNH BÁO QUAN TRỌNG: GPIO 6-11 nối trực tiếp vào SPI Flash nội bộ, cấm sử
// dụng!
constexpr uint8_t SERVO_L_PIN = 4; // GPIO 4
constexpr uint8_t SERVO_R_PIN = 5; // GPIO 5
constexpr uint8_t LED_PIN = 2;     // Onboard LED của ESP32 Dev Module (GPIO 2)
#endif

// Wi-Fi credentials — change these to match your network.
// Leave SSID empty ("") to always run in SoftAP mode.
constexpr char WIFI_STA_SSID[] = "Phong12";
constexpr char WIFI_STA_PASS[] = "abcd1234";
constexpr char WIFI_AP_SSID[] = "Phelan-Butterfly";
constexpr char WIFI_AP_PASS[] = ""; // Mật khẩu SoftAP (để trống "" để mở tự do không cần mật khẩu, hoặc điền tối thiểu 8 ký tự)
constexpr char MDNS_HOST[] = "phelan";

constexpr int SERVO_HZ = 300;
constexpr int SERVO_CENTER = 90;
constexpr int SERVO_MIN_ANGLE = 20;
constexpr int SERVO_MAX_ANGLE = 160;
constexpr int SERVO_EMA_ALPHA_SHIFT = 1;
constexpr bool REVERSE_SERVO_L = false;
constexpr bool REVERSE_SERVO_R = false;

constexpr int THROTTLE_IDLE_MAX = 1050;
constexpr unsigned long LOOP_DT_US = 3333;
constexpr int FLAP_AMP = 65;
constexpr uint32_t FLAP_SPEED_MIN = 157286;
constexpr uint32_t FLAP_SPEED_MAX = 262144;
constexpr int PERCH_MULT = 31;
constexpr int PERCH_BASE_UP_ANGLE = 30;
constexpr int FAILSAFE_DIHEDRAL = 15;
constexpr int GLIDE_SERVO_SPEED_DPS = 1570;

constexpr int16_t PERCH_ENTER_ELEV = 1300;
constexpr int16_t PERCH_EXIT_ELEV = 1700;
constexpr int16_t STICK_DEADBAND = 10;
constexpr unsigned long WS_FAILSAFE_MS = 500;
constexpr unsigned long RX_HARD_FAILSAFE_MS = 3000;

constexpr int16_t CH_CENTER = 1500;
constexpr int16_t CH_MIN = 1000;
constexpr int16_t CH_MAX = 2000;

constexpr uint8_t CH_STEERING = 0;
constexpr uint8_t CH_ELEVATOR = 1;
constexpr uint8_t CH_THROTTLE = 2;
constexpr uint8_t CH_YAW = 3;
constexpr uint8_t CH_KILL_SWITCH = 4;
constexpr int16_t KILL_SWITCH_THRESHOLD = 1700;
constexpr int PERCH_HEADROOM_DEG = 15;

inline int mapTrim15(int x) {
  int d = x - CH_CENTER;
  if (abs(d) <= STICK_DEADBAND)
    return 0;
  int s = (d > 0) ? (d - STICK_DEADBAND) : (d + STICK_DEADBAND);
  return constrain(s / 30, -15, 15);
}
} // namespace Config

#pragma pack(push, 1)
struct FlightPacket {
  uint8_t header;
  uint8_t flags;
  int8_t throttle;
  int8_t yaw;
  int8_t pitch;
  int8_t roll;
};

struct TelemetryPacket {
  uint8_t header;
  uint8_t flags;
  int8_t offset;
  uint8_t servoL;
  uint8_t servoR;
  uint8_t cpuTemp;
  uint8_t cpuMhz;
  uint16_t uptimeSec;
  uint8_t timeH;
  uint8_t timeM;
  uint8_t timeS;
};
#pragma pack(pop)

struct alignas(4) ControlSnapshot {
  int16_t ch[4];
  uint8_t flags;
};

enum class FlightMode { INIT, CONNECTED, PERCHING, FAILSAFE };
