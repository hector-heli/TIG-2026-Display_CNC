#include <Arduino.h>

#define LED_R 4
#define LED_G 16
#define LED_B 17

static uint8_t ledR=0, ledG=0, ledB=0;

void ledBegin() {
  ledcAttach(LED_R, 5000, 8);
  ledcAttach(LED_G, 5000, 8);
  ledcAttach(LED_B, 5000, 8);
  ledcWrite(LED_R, 0);
  ledcWrite(LED_G, 0);
  ledcWrite(LED_B, 0);
}

void ledSet(uint8_t r, uint8_t g, uint8_t b) {
  if (r==ledR && g==ledG && b==ledB) return;
  ledR=r; ledG=g; ledB=b;
  ledcWrite(LED_R, r);
  ledcWrite(LED_G, g);
  ledcWrite(LED_B, b);
}

void ledUpdate() {
  // Prioridad: alarma > homing > sin enlace > ok
  if (state.alarm) {
    // rojo
    static unsigned long lastBlink=0;
    unsigned long now=millis();
    if (now-lastBlink>250) {
      lastBlink=now;
      static bool on=false;
      on=!on;
      ledSet(on?255:0, 0, 0);
    }
    return;
  }
  if (state.homing) {
    // cian
    ledSet(0, 255, 255);
    return;
  }
  if (!state.linkOk) {
    // amarillo intermitente
    static unsigned long lastBlink=0;
    unsigned long now=millis();
    if (now-lastBlink>500) {
      lastBlink=now;
      static bool on=false;
      on=!on;
      ledSet(on?255:0, on?200:0, 0);
    }
    return;
  }
  // conectado - verde tenue
  ledSet(0, 40, 0);
}
