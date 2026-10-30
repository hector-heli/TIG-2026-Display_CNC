#include <Arduino.h>
#include "LogoUAN.h"

#define SPLASH_MS 5000
#define UAN_BLUE 0x01B8  // aproximado rgb(6, 53, 196)

static unsigned long splashUntil = 0;
static bool splashActive = false;

void splashShow() {
  splashActive = true;
  splashUntil = millis() + SPLASH_MS;
  
  tft.fillScreen(TFT_WHITE);
  
  // Logo centrado
  int x = (SCREEN_W - LOGO_UAN_W) / 2;
  int y = 10;
  tft.pushImage(x, y, LOGO_UAN_W, LOGO_UAN_H, logoUAN);
  
  // Textos
  tft.setTextColor(UAN_BLUE, TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(2);
  tft.drawString("Universidad Antonio Narino - FIMEB", SCREEN_W/2, y + LOGO_UAN_H + 18);
  
  tft.setTextFont(2);
  tft.drawString("Hector Heli Ruiz Garcia", SCREEN_W/2, y + LOGO_UAN_H + 42);
  
  tft.setTextFont(1);
  tft.drawString("Ing Adriana Lopez Vargas", SCREEN_W/2, y + LOGO_UAN_H + 62);
  tft.drawString("Ing Jose Rodrigo Munoz", SCREEN_W/2, y + LOGO_UAN_H + 78);
}

bool splashIsActive() {
  return splashActive;
}

void splashUpdate() {
  if (!splashActive) return;
  if (millis() >= splashUntil) {
    splashActive = false;
    state.page = PAGE_MAIN;
    state.pageDirty = true;
    uiDraw();
  }
}
