#include <SPIFFS.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_system.h>

#include "Config.h"

TFT_eSPI tft = TFT_eSPI();

PendantState state;
CommandState cmd;
HardwareSerial LinkSerial(1);
Preferences prefs;

void settingsLoad();
void settingsSave();
void settingsSaveLink();
void touchCalibrate(bool force);
void linkBegin();
void linkPoll();
void grblBegin();
void grblProcess();
bool grblSendCommand(const char *text, bool motion = false);
bool boxGenerateFile();
bool jobStart();
void jobPause();
void jobResume();
void jobStop();
void jobProcess();
void jobOnGrblError(const char *reason);
void jobAbortM5();
void jobAbortReset();
void jobAbortDismiss();
void uiBegin();
void uiUpdate();
void uiHandleTouch(int16_t x, int16_t y);
void backlightBegin();
void backlightUpdate(bool touched);
void backlightWake();
bool backlightIsOn();
void ledBegin();
void ledUpdate();
void splashShow();
void splashUpdate();
bool splashIsActive();

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.printf("\nCYD pendant boot reset=%d\n", (int)esp_reset_reason());

  tft.init();
  tft.setRotation(DISPLAY_ROTATION);
  tft.fillScreen(TFT_BLACK);
  backlightBegin();

  ledBegin();

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed");
  }

  settingsLoad();
  touchCalibrate(false);

  linkBegin();
  grblBegin();
  
  splashShow();
  linkPoll();

}

void loop() {
  if (splashIsActive()) {
    linkPoll();
    grblProcess();
    splashUpdate();
    ledUpdate();
    delay(5);
    return;
  }
  
  linkPoll();
  grblProcess();
  jobProcess();

  bool pressed = tft.getTouch(&state.touchX, &state.touchY);
  if (pressed && !state.touchDown) {
    state.touchDown = true;
    if (backlightIsOn()) {
      uiHandleTouch(state.touchX, state.touchY);
    } else {
      backlightWake();
    }
  } else if (!pressed && state.touchDown) {
    state.touchDown = false;
  }

  backlightUpdate(pressed);
  if (backlightIsOn()) uiUpdate();
  ledUpdate();
  delay(5);
}
