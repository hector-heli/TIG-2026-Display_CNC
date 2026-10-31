#include <Arduino.h>

#define LED_R 4
#define LED_G 16
#define LED_B 17
#define LED_ERROR_HOLD_MS 6000

static uint8_t ledR = 0, ledG = 0, ledB = 0;

static uint8_t ledDuty(uint8_t level) {
#if RGB_LED_ACTIVE_LOW
  return 255 - level;
#else
  return level;
#endif
}

static bool ledBlink(unsigned long periodMs) {
  static unsigned long lastBlinkMs = 0;
  static bool on = false;
  unsigned long now = millis();

  if (now - lastBlinkMs >= periodMs) {
    lastBlinkMs = now;
    on = !on;
  }
  return on;
}

void ledBegin() {
  ledcAttach(LED_R, 5000, 8);
  ledcAttach(LED_G, 5000, 8);
  ledcAttach(LED_B, 5000, 8);
  ledcWrite(LED_R, ledDuty(0));
  ledcWrite(LED_G, ledDuty(0));
  ledcWrite(LED_B, ledDuty(0));
}

void ledSet(uint8_t r, uint8_t g, uint8_t b) {
  if (r == ledR && g == ledG && b == ledB) return;
  ledR = r;
  ledG = g;
  ledB = b;
  ledcWrite(LED_R, ledDuty(r));
  ledcWrite(LED_G, ledDuty(g));
  ledcWrite(LED_B, ledDuty(b));
}

void ledUpdate() {
  unsigned long now = millis();
  bool errorRecent = state.lastError[0] != 0 &&
                     state.lastErrorMs != 0 &&
                     now - state.lastErrorMs < LED_ERROR_HOLD_MS;

  if (state.alarm) {
    bool on = ledBlink(250);
    ledSet(on ? 255 : 0, 0, 0);
    return;
  }

  if (state.jobAbortPrompt || state.jobState == JOB_ERROR) {
    ledSet(255, 0, 0);
    return;
  }

  if (errorRecent) {
    ledSet(180, 0, 0);
    return;
  }

  if (state.homing) {
    ledSet(0, 255, 255);
    return;
  }

  if (!state.linkOk) {
    bool on = ledBlink(500);
    ledSet(on ? 255 : 0, on ? 180 : 0, 0);
    return;
  }

  if (state.jobState == JOB_PAUSED ||
      strncmp(state.grblState, "Hold", 4) == 0) {
    ledSet(255, 120, 0);
    return;
  }

  if (state.jobState == JOB_RUNNING ||
      strncmp(state.grblState, "Run", 3) == 0 ||
      strncmp(state.grblState, "Jog", 3) == 0) {
    ledSet(0, 180, 0);
    return;
  }

  ledSet(0, 0, 40);
}
