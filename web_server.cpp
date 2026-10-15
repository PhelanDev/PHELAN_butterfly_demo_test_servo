#include "web_server.h"
#include "flight.h"
#include "servo_sys.h"
#include "web_ui.h"
#include <ESPmDNS.h>
#include <WiFi.h>

namespace WebServer {
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

bool wifiActive = false;
bool webControlEnabled = false;
volatile unsigned long lastWsPacketTime = 0;
unsigned long lastTelemSendTime = 0;
volatile int8_t wsArmRequest = 0;
volatile int8_t wsPerchRequest = 0;
constexpr unsigned long TELEM_INTERVAL_MS = 200;

static int16_t webChannels[16] = {1500, 1500, 1000, 1500, 1000, 1500, 1500, 1500,
                                  1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500};

static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    if (client->client())
      client->client()->setNoDelay(true);
  } else if (type == WS_EVT_DISCONNECT) {
    if (ws.count() == 0)
      webControlEnabled = false;
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if (info->opcode == WS_BINARY && len == sizeof(FlightPacket) &&
        data[0] == 0xAA) {
      FlightPacket *pkt = (FlightPacket *)data;
      lastWsPacketTime = millis();

      webChannels[Config::CH_THROTTLE] =
          constrain(1000 + (int16_t)pkt->throttle * 10, 1000, 2000);
      webChannels[Config::CH_YAW] =
          constrain(1500 + (int16_t)pkt->yaw * 5, 1000, 2000);
      webChannels[Config::CH_ELEVATOR] =
          constrain(1500 + (int16_t)pkt->pitch * 5, 1000, 2000);
      webChannels[Config::CH_STEERING] =
          constrain(1500 + (int16_t)pkt->roll * 5, 1000, 2000);

      wsArmRequest = (pkt->flags & 0x01) ? 1 : 0;
      webChannels[Config::CH_KILL_SWITCH] = (pkt->flags & 0x02) ? 1900 : 1000;
      wsPerchRequest = (pkt->flags & 0x04) ? 1 : 0;

      webControlEnabled = true;
    } else if (info->opcode == WS_TEXT && len >= 4 &&
               memcmp(data, "PING", 4) == 0) {
      client->text("PONG");
    }
  }
}

void sendTelemetry() {
  if (ws.count() == 0)
    return;

  TelemetryPacket tpkt;
  tpkt.header = 0xBB;
  uint8_t modeId = 0;
  switch (Flight::state.mode) {
  case FlightMode::CONNECTED:
    modeId = 1;
    break;
  case FlightMode::PERCHING:
    modeId = 2;
    break;
  case FlightMode::FAILSAFE:
    modeId = 3;
    break;
  default:
    modeId = 0;
    break;
  }
  tpkt.flags = (Flight::state.isArmed ? 1 : 0) | (modeId << 1);
  tpkt.offset = (int8_t)constrain((int)Flight::state.currentOffset, -127, 127);
  tpkt.servoL = (uint8_t)constrain(ServoSystem::state.lastPosL, 0, 255);
  tpkt.servoR = (uint8_t)constrain(ServoSystem::state.lastPosR, 0, 255);
#if defined(CONFIG_IDF_TARGET_ESP32) || defined(ESP32)
  tpkt.cpuTemp = (uint8_t)constrain((int)temperatureRead(), 0, 120);
  tpkt.cpuMhz = (uint8_t)getCpuFrequencyMhz();
#else
  tpkt.cpuTemp = 42;
  tpkt.cpuMhz = 160;
#endif
  uint32_t upSec = millis() / 1000;
  tpkt.uptimeSec = (uint16_t)upSec;
  tpkt.timeH = (uint8_t)((upSec / 3600) % 24);
  tpkt.timeM = (uint8_t)((upSec / 60) % 60);
  tpkt.timeS = (uint8_t)(upSec % 60);

  ws.binaryAll((uint8_t *)&tpkt, sizeof(tpkt));
}

bool isConnected() {
  unsigned long now = millis();
  bool linkLost =
      (lastWsPacketTime == 0 || (now - lastWsPacketTime > Config::WS_FAILSAFE_MS));
  return !linkLost && webControlEnabled;
}

void init() {
  Serial.println();
  Serial.println("[BOOT] Khoi dong he thong mang Wi-Fi...");

  IPAddress apIP(192, 168, 4, 1);
  IPAddress netMsk(255, 255, 255, 0);

  bool hasStaConfig = (strlen(Config::WIFI_STA_SSID) > 0 &&
                       strcmp(Config::WIFI_STA_SSID, "YourWiFiSSID") != 0);

  if (hasStaConfig) {
    WiFi.mode(WIFI_AP_STA);
  } else {
    WiFi.mode(WIFI_AP);
  }

  WiFi.softAPConfig(apIP, apIP, netMsk);
  const char *apSsid = (strlen(Config::WIFI_AP_SSID) > 0)
                           ? Config::WIFI_AP_SSID
                           : "Phelan-Butterfly";
  const char *apPass =
      (strlen(Config::WIFI_AP_PASS) >= 8) ? Config::WIFI_AP_PASS : nullptr;
  WiFi.softAP(apSsid, apPass);

  Serial.printf("[WiFi] SoftAP San sang! SSID: %s | IP: http://%s\n", apSsid,
                WiFi.softAPIP().toString().c_str());

  bool staConnected = false;
  if (hasStaConfig) {
    Serial.printf("[WiFi] Dang ket noi Router: %s ...\n", Config::WIFI_STA_SSID);
    WiFi.begin(Config::WIFI_STA_SSID, Config::WIFI_STA_PASS);

    unsigned long startWait = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startWait < 8000)) {
      delay(250);
      Serial.print(".");
      digitalWrite(Config::LED_PIN, !digitalRead(Config::LED_PIN));
    }

    staConnected = (WiFi.status() == WL_CONNECTED);
    if (staConnected) {
      Serial.printf("\n[WiFi] Da ket noi Router thanh cong! IP: http://%s | Ch: %d\n",
                    WiFi.localIP().toString().c_str(), WiFi.channel());
    } else {
      Serial.println(
          "\n[WiFi] Khong the ket noi Router! Dang chay doc lap bang SoftAP.");
    }
  }

  if (MDNS.begin(Config::MDNS_HOST)) {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("[mDNS] http://%s.local\n", Config::MDNS_HOST);
  }

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html; charset=utf-8", (const uint8_t *)PAGE_HTML,
                    sizeof(PAGE_HTML) - 1);
  });
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html; charset=utf-8", (const uint8_t *)PAGE_HTML,
                    sizeof(PAGE_HTML) - 1);
  });

  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/");
  });
  server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect("/");
  });

  WiFi.setSleep(false);
  server.begin();
  wifiActive = true;
  digitalWrite(Config::LED_PIN, LOW);
  Serial.println("[HTTP] Web Server da khoi dong thanh cong tren cong 80!");
}

void handleLoop() {
  unsigned long now = millis();

  bool linkLost =
      (lastWsPacketTime == 0 || (now - lastWsPacketTime > Config::WS_FAILSAFE_MS));
  if (linkLost && webControlEnabled) {
    webControlEnabled = false;
  }

  bool snapshotConnected = !linkLost && webControlEnabled;
  bool killActive =
      webChannels[Config::CH_KILL_SWITCH] > Config::KILL_SWITCH_THRESHOLD;
  bool armRequested = (wsArmRequest == 1);

  uint8_t flags = 0;
  if (snapshotConnected)
    flags |= 0x01;
  if (killActive)
    flags |= 0x02;
  if (armRequested)
    flags |= 0x04;
  if (wsPerchRequest == 1)
    flags |= 0x08;

  publishControlSnapshot(
      webChannels[Config::CH_STEERING], webChannels[Config::CH_ELEVATOR],
      webChannels[Config::CH_THROTTLE], webChannels[Config::CH_YAW], flags);

  if (now - lastTelemSendTime >= TELEM_INTERVAL_MS) {
    lastTelemSendTime = now;
    sendTelemetry();
    ws.cleanupClients();
  }
}
}
