#pragma once

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Link mode
// ---------------------------------------------------------------------------

enum LinkMode : uint8_t {
  LINK_UART = 0,
  LINK_WIFI = 1
};

#define LINK_MODE_DEFAULT LINK_UART

#define WIFI_RECONNECT_MS 5000
#define WIFI_CONNECT_TIMEOUT_MS 15000

// ---------------------------------------------------------------------------
// WiFi defaults
// ---------------------------------------------------------------------------

#define WIFI_DEFAULT_SSID "MKS_DLC"
#define WIFI_DEFAULT_PASS "12345678"

// ---------------------------------------------------------------------------
// DLC32 TCP defaults (DLC32 en AP mode suele usar 192.168.4.1)
// ---------------------------------------------------------------------------

#define DLC32_DEFAULT_IP "192.168.4.1"
#define DLC32_DEFAULT_PORT 80

// ---------------------------------------------------------------------------
// CYD <-> Controller link (UART fallback)
// ---------------------------------------------------------------------------

#define CYD_RX_PIN 22
#define CYD_TX_PIN 21
#define LINK_BAUD 115200

#define STATUS_POLL_MS 250
#define LINK_TIMEOUT_MS 1200
#define CMD_TIMEOUT_MS 2000

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

#define DISPLAY_ROTATION 1
#define CALIBRATION_FILE "/TouchCalData2"

#define SCREEN_W 480
#define SCREEN_H 320

// ---------------------------------------------------------------------------
// Defaults
// ---------------------------------------------------------------------------

#define DEFAULT_JOG_STEP 1.0f
#define DEFAULT_JOG_FEED 500.0f
#define DEFAULT_MAX_RPM 1000
#define DEFAULT_TARGET_RPM 500

// ---------------------------------------------------------------------------
// UI / Splash
// ---------------------------------------------------------------------------

#define SPLASH_DURATION_MS 5000

// ---------------------------------------------------------------------------
// Shared state
// ---------------------------------------------------------------------------

enum Page : uint8_t {
  PAGE_MAIN = 0,
  PAGE_SPINDLE,
  PAGE_SETTINGS
};

enum SpindleDir : uint8_t {
  SPINDLE_STOP = 0,
  SPINDLE_CW,
  SPINDLE_CCW
};

enum ConfirmAction : uint8_t {
  CONFIRM_NONE = 0,
  CONFIRM_HOME,
  CONFIRM_ZERO_ALL,
  CONFIRM_SPINDLE_CW,
  CONFIRM_SPINDLE_CCW,
  CONFIRM_ALARM_RESET,
  CONFIRM_TOUCH_CAL
};

struct PendantState {
  char grblState[12];
  bool alarm;
  bool homing;
  bool linkOk;

  bool wposValid;
  float mpos[3];
  float wpos[3];

  float feed;
  float spindle;

  int feedOv;
  int spindleOv;
  int rapidOv;

  unsigned long lastStatusMs;
  unsigned long lastRxMs;

  char message[48];
  unsigned long messageMs;

  char lastError[40];
  unsigned long lastErrorMs;

  Page page;

  float jogStep;
  float jogFeed;
  int maxRpm;
  int targetRpm;
  SpindleDir spindleDir;

  bool statusDirty;
  bool pageDirty;

  bool touchDown;
  uint16_t touchX;
  uint16_t touchY;

  ConfirmAction confirm;
  unsigned long confirmMs;

  // Link configuration
  LinkMode linkMode;
  char wifiSsid[33];
  char wifiPass[33];
  char dlcIp[16];
  uint16_t dlcPort;
  bool wifiConnected;
  bool tcpConnected;
  unsigned long lastWifiReconnectMs;
};

struct CommandState {
  char text[48];
  bool busy;
  bool motion;
  unsigned long sentMs;
};

extern PendantState state;
extern CommandState cmd;
extern HardwareSerial LinkSerial;
