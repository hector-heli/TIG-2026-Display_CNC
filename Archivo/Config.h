#pragma once

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Link mode
// ---------------------------------------------------------------------------

enum LinkMode : uint8_t {
  LINK_UART = 0,
  LINK_WIFI = 1
};

#define LINK_MODE_DEFAULT LINK_WIFI

#define WIFI_RECONNECT_MS 5000
#define WIFI_CONNECT_TIMEOUT_MS 15000

// ---------------------------------------------------------------------------
// WiFi defaults
// ---------------------------------------------------------------------------

#define WIFI_DEFAULT_SSID "MKS_DLC"
#define WIFI_DEFAULT_PASS "12345678"

// ---------------------------------------------------------------------------
// DLC32 TCP defaults (DLC32 en AP mode suele usar 192.168.4.1)
// El puerto 23 es el Telnet/GRBL del DLC32; 8080 se mantiene como
// alternativa para otras variantes de firmware. El puerto 80 es la
// interfaz web HTTP y no entrega el flujo GRBL.
// ---------------------------------------------------------------------------

#define DLC32_DEFAULT_IP "192.168.4.1"
#define DLC32_DEFAULT_PORT 23
#define DLC32_ALT_PORT 8080
#define DLC32_HTTP_PORT 80

// ---------------------------------------------------------------------------
// CYD <-> Controller link (UART fallback)
// ---------------------------------------------------------------------------

#define CYD_RX_PIN 22
#define CYD_TX_PIN 21
#define LINK_BAUD 115200

#define STATUS_POLL_MS 250
#define LINK_TIMEOUT_MS 1200
#define TCP_STALE_MS 3000
#define TCP_RETRY_COOLDOWN_MS 30000UL
#define CMD_TIMEOUT_MS 2000
#define JOB_ACK_TIMEOUT_MS 5000

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

#define DISPLAY_ROTATION 1
#define CALIBRATION_FILE "/TouchCalData2"
#define BACKLIGHT_TIMEOUT_MS 180000UL
#define RGB_LED_ACTIVE_LOW 1

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
// Caja / desbaste
// ---------------------------------------------------------------------------

#define BOX_DEFAULT_LENGTH 40.0f
#define BOX_DEFAULT_WIDTH 30.0f
#define BOX_DEFAULT_HEIGHT 10.0f
#define BOX_DEFAULT_TOOL_DIA 6.0f
#define BOX_DEFAULT_STEPOVER_PCT 40.0f
#define BOX_DEFAULT_FEED 400.0f
#define BOX_DEFAULT_RPM 900
#define BOX_DEFAULT_SAFE_Z 10.0f
#define BOX_DEFAULT_DEPTH_STEP 1.0f
#define BOX_DEFAULT_PLUNGE_FEED 200.0f
#define BOX_MAX_LINES 20000
#define BOX_FILE_PATH "/box.nc"

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
  PAGE_BOX,
  PAGE_SETTINGS
};

enum JobState : uint8_t {
  JOB_IDLE = 0,
  JOB_RUNNING,
  JOB_PAUSED,
  JOB_ERROR
};

enum BoxField : uint8_t {
  BOX_FIELD_NONE = 0,
  BOX_FIELD_LENGTH,
  BOX_FIELD_WIDTH,
  BOX_FIELD_HEIGHT,
  BOX_FIELD_TOOL,
  BOX_FIELD_STEPOVER,
  BOX_FIELD_FEED,
  BOX_FIELD_RPM,
  BOX_FIELD_SAFE_Z,
  BOX_FIELD_DEPTH_STEP,
  BOX_FIELD_PLUNGE
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
  CONFIRM_TOUCH_CAL,
  CONFIRM_START_JOB,
  CONFIRM_STOP_JOB,
  CONFIRM_ABORT_RESET
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

  // Caja / desbaste
  float boxLength;
  float boxWidth;
  float boxHeight;
  float boxToolDia;
  float boxStepoverPct;
  float boxFeed;
  int boxRpm;
  float boxSafeZ;
  float boxDepthStep;
  float boxPlungeFeed;

  JobState jobState;
  uint16_t jobTotalLines;
  uint16_t jobLine;
  bool jobDirty;
  bool jobAbortPrompt;
};

struct CommandState {
  char text[96];
  bool busy;
  bool motion;
  unsigned long sentMs;
  bool spindlePending;
  SpindleDir pendingSpindleDir;
};

extern PendantState state;
extern CommandState cmd;
extern HardwareSerial LinkSerial;
