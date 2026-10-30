#include <SPIFFS.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Preferences.h>
#include <WiFi.h>

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
void uiBegin();
void uiUpdate();
void uiHandleTouch(int16_t x, int16_t y);
void ledBegin();
void ledUpdate();
void splashShow();
void splashUpdate();
bool splashIsActive();

void setup() {
  Serial.begin(115200);
  delay(100);

  tft.init();
  tft.setRotation(DISPLAY_ROTATION);
  tft.fillScreen(TFT_BLACK);

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
    splashUpdate();
    ledUpdate();
    delay(5);
    return;
  }
  
  linkPoll();
  grblProcess();

  bool pressed = tft.getTouch(&state.touchX, &state.touchY);
  if (pressed && !state.touchDown) {
    state.touchDown = true;
    uiHandleTouch(state.touchX, state.touchY);
  } else if (!pressed && state.touchDown) {
    state.touchDown = false;
  }

  uiUpdate();
  ledUpdate();
  delay(5);
}
