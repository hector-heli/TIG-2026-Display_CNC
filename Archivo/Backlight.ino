#include <Arduino.h>

static bool backlightOn = true;
static unsigned long lastActivityMs = 0;
static unsigned long lastErrorSeenMs = 0;
static bool lastAlarm = false;
static bool lastAbortPrompt = false;

static void backlightSet(bool on) {
  if (backlightOn == on) return;
  digitalWrite(TFT_BL, on ? TFT_BACKLIGHT_ON : !TFT_BACKLIGHT_ON);
  backlightOn = on;
}

void backlightBegin() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  backlightOn = true;
  lastActivityMs = millis();
  lastErrorSeenMs = state.lastErrorMs;
  lastAlarm = state.alarm;
  lastAbortPrompt = state.jobAbortPrompt;
}

bool backlightIsOn() {
  return backlightOn;
}

void backlightWake() {
  backlightSet(true);
  lastActivityMs = millis();
  state.pageDirty = true;
  state.statusDirty = true;
  state.jobDirty = true;
}

void backlightUpdate(bool touched) {
  unsigned long now = millis();

  if (state.lastErrorMs != lastErrorSeenMs) {
    lastErrorSeenMs = state.lastErrorMs;
    if (state.lastError[0] != 0) {
      lastActivityMs = now;
      backlightWake();
    }
  }

  if (state.alarm != lastAlarm) {
    lastAlarm = state.alarm;
    if (state.alarm) {
      lastActivityMs = now;
      backlightWake();
    }
  }

  if (state.jobAbortPrompt != lastAbortPrompt) {
    lastAbortPrompt = state.jobAbortPrompt;
    if (state.jobAbortPrompt) {
      lastActivityMs = now;
      backlightWake();
    }
  }

  if (state.alarm || state.jobAbortPrompt || state.jobState == JOB_ERROR) {
    lastActivityMs = now;
    backlightSet(true);
    return;
  }

  if (touched) {
    lastActivityMs = now;
    backlightSet(true);
    return;
  }

  if (backlightOn && now - lastActivityMs >= BACKLIGHT_TIMEOUT_MS) {
    backlightSet(false);
  }
}
