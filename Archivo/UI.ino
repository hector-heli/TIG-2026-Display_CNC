#define TOP_H 28
#define NAV_Y 286
#define NAV_H 32

#define DRO_X 6
#define DRO_Y 31
#define DRO_W 468
#define DRO_H 72

#define INFO_Y 108
#define STEP_Y 128
#define STEP_H 28
#define JOG_Y 160
#define JOG_ROW_H 32
#define JOG_ROW_GAP 4
#define RIGHT_X 276
#define RIGHT_W 198

static const float stepValues[4] = {0.1f, 1.0f, 10.0f, 100.0f};
static const char *stepLabels[4] = {"0.1", "1", "10", "100"};
static const char *axisNames[3] = {"X", "Y", "Z"};

void uiDrawButton(int16_t x, int16_t y, int16_t w, int16_t h,
                  const char *label, uint16_t fill, bool selected = false,
                  uint8_t font = 2) {
  tft.fillRect(x, y, w, h, fill);
  tft.drawRect(x, y, w, h, selected ? TFT_WHITE : TFT_DARKGREY);
  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(font);
  tft.setTextColor(TFT_WHITE, fill);
  tft.drawString(label, x + w / 2, y + h / 2);
}

void uiDrawStatusBar() {
  tft.fillRect(0, 0, SCREEN_W, TOP_H, TFT_NAVY);

  tft.setTextDatum(ML_DATUM);
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.drawString("CYD PENDANT", 6, TOP_H / 2);

  uint16_t stateColor = TFT_WHITE;
  if (state.alarm) stateColor = TFT_RED;
  else if (state.homing) stateColor = TFT_CYAN;

  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(stateColor, TFT_NAVY);
  tft.drawString(state.grblState, SCREEN_W / 2, TOP_H / 2);

  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(state.linkOk ? TFT_GREEN : TFT_RED, TFT_NAVY);
  tft.drawString(state.linkOk ? "LINK" : "SIN LINK", SCREEN_W - 6, TOP_H / 2);
}

void uiDrawDro() {
  const float *pos = state.wposValid ? state.wpos : state.mpos;

  tft.fillRect(DRO_X, DRO_Y, DRO_W, DRO_H, TFT_BLACK);
  tft.drawRect(DRO_X, DRO_Y, DRO_W, DRO_H, TFT_DARKGREY);

  for (uint8_t i = 0; i < 3; i++) {
    int16_t cellX = DRO_X + i * 156;
    if (i > 0) tft.drawFastVLine(cellX, DRO_Y + 4, DRO_H - 8, TFT_DARKGREY);

    tft.setTextDatum(TC_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString(axisNames[i], cellX + 78, DRO_Y + 5);

    char buf[20];
    snprintf(buf, sizeof(buf), "%.2f", pos[i]);
    tft.setTextFont(4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(buf, cellX + 78, DRO_Y + 30);
  }
}

void uiDrawInfo() {
  tft.fillRect(6, INFO_Y, SCREEN_W - 12, 16, TFT_BLACK);

  tft.setTextDatum(ML_DATUM);
  tft.setTextFont(2);

  char buf[80];
  if (state.message[0] != 0 && millis() - state.messageMs < 4000) {
    snprintf(buf, sizeof(buf), "%s", state.message);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  } else {
    snprintf(buf, sizeof(buf), "F %.0f  S %.0f  Ov %d/%d/%d  %s",
             state.feed, state.spindle, state.feedOv, state.rapidOv,
             state.spindleOv, state.wposValid ? "WPos" : "MPos");
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  }

  tft.drawString(buf, 8, INFO_Y + 8);
}

void uiDrawStepRow() {
  tft.fillRect(6, STEP_Y, 310, STEP_H, TFT_BLACK);

  tft.setTextDatum(ML_DATUM);
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Paso", 8, STEP_Y + STEP_H / 2);

  for (uint8_t i = 0; i < 4; i++) {
    int16_t x = 54 + i * 64;
    bool selected = fabsf(state.jogStep - stepValues[i]) < 0.001f;
    uiDrawButton(x, STEP_Y, 60, STEP_H, stepLabels[i],
                 selected ? TFT_BLUE : TFT_DARKGREY, selected);
  }
}

void uiDrawJog() {
  for (uint8_t i = 0; i < 3; i++) {
    int16_t rowY = JOG_Y + i * (JOG_ROW_H + JOG_ROW_GAP);

    tft.setTextDatum(ML_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(axisNames[i], 8, rowY + JOG_ROW_H / 2);

    uiDrawButton(28, rowY, 76, JOG_ROW_H, "-", TFT_DARKGREEN);
    uiDrawButton(108, rowY, 76, JOG_ROW_H, "+", TFT_DARKGREEN);
    uiDrawButton(188, rowY, 76, JOG_ROW_H, "0", TFT_DARKGREY);
  }

  uiDrawButton(RIGHT_X, 160, RIGHT_W, 22, "Home All", TFT_DARKGREY, false, 2);
  uiDrawButton(RIGHT_X, 184, RIGHT_W, 22, "Hold", TFT_ORANGE, false, 2);
  uiDrawButton(RIGHT_X, 208, RIGHT_W, 22, "Resume", TFT_DARKGREEN, false, 2);
  uiDrawButton(RIGHT_X, 232, RIGHT_W, 22, "Zero All", TFT_DARKGREY, false, 2);
  uiDrawButton(RIGHT_X, 256, RIGHT_W, 22, "Alarm $X", TFT_MAROON, false, 2);
}

void uiDrawNav() {
  static const char *labels[3] = {"Principal", "Spindle", "Ajustes"};

  for (uint8_t i = 0; i < 3; i++) {
    int16_t x = 6 + i * 158;
    bool selected = (state.page == (Page)i);
    uiDrawButton(x, NAV_Y, 154, NAV_H, labels[i],
                 selected ? TFT_BLUE : TFT_DARKGREY, selected);
  }
}

void uiDrawMain() {
  uiDrawStatusBar();
  tft.fillRect(0, TOP_H, SCREEN_W, NAV_Y - TOP_H, TFT_BLACK);
  uiDrawDro();
  uiDrawInfo();
  uiDrawStepRow();
  uiDrawJog();
  uiDrawNav();
}

void uiDrawSpindleInfo() {
  tft.fillRect(6, 238, SCREEN_W - 12, 40, TFT_BLACK);
  tft.setTextDatum(ML_DATUM);
  tft.setTextFont(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

  char buf[80];
  snprintf(buf, sizeof(buf), "Actual  F %.0f  S %.0f   Objetivo %d RPM",
           state.feed, state.spindle, state.targetRpm);
  tft.drawString(buf, 8, 248);

  tft.setTextFont(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("M3/M4 aplican el objetivo. M5 detiene. Overrides actúan al instante.",
                 8, 266);
}

void uiDrawSpindle() {
  uiDrawStatusBar();
  tft.fillRect(0, TOP_H, SCREEN_W, NAV_Y - TOP_H, TFT_BLACK);

  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("SPINDLE", SCREEN_W / 2, 38);

  tft.fillRect(6, 48, 468, 56, TFT_BLACK);
  tft.drawRect(6, 48, 468, 56, TFT_DARKGREY);

  tft.setTextDatum(ML_DATUM);
  tft.setTextFont(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("RPM objetivo", 16, 62);
  tft.setTextFont(1);
  tft.drawString("Max:", 404, 62);

  char buf[24];
  snprintf(buf, sizeof(buf), "%d", state.targetRpm);
  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(6);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(buf, SCREEN_W / 2, 82);

  snprintf(buf, sizeof(buf), "%d", state.maxRpm);
  tft.setTextDatum(MR_DATUM);
  tft.setTextFont(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString(buf, 466, 62);

  uiDrawButton(6, 112, 72, 32, "-50", TFT_DARKGREY);
  uiDrawButton(84, 112, 72, 32, "+50", TFT_DARKGREY);
  uiDrawButton(162, 112, 72, 32, "500", TFT_DARKGREY);
  uiDrawButton(240, 112, 72, 32, "1000", TFT_DARKGREY);
  uiDrawButton(318, 112, 72, 32, "M3 CW",
               state.spindleDir == SPINDLE_CW ? TFT_DARKGREEN : TFT_DARKGREY,
               state.spindleDir == SPINDLE_CW);
  uiDrawButton(396, 112, 72, 32, "M4 CCW",
               state.spindleDir == SPINDLE_CCW ? TFT_DARKGREEN : TFT_DARKGREY,
               state.spindleDir == SPINDLE_CCW);

  uiDrawButton(6, 154, 72, 32, "OFF",
               state.spindleDir == SPINDLE_STOP ? TFT_MAROON : TFT_DARKGREY,
               state.spindleDir == SPINDLE_STOP);
  uiDrawButton(84, 154, 72, 32, "Feed +", TFT_DARKGREY);
  uiDrawButton(162, 154, 72, 32, "Feed -", TFT_DARKGREY);
  uiDrawButton(240, 154, 72, 32, "Feed 100", TFT_DARKGREY);
  uiDrawButton(318, 154, 72, 32, "Spindle +", TFT_DARKGREY, false, 1);
  uiDrawButton(396, 154, 72, 32, "Spindle -", TFT_DARKGREY, false, 1);

  uiDrawButton(6, 196, 150, 32, "Spindle 100", TFT_DARKGREY);
  uiDrawButton(162, 196, 150, 32, "Rapid 100", TFT_DARKGREY);
  uiDrawButton(318, 196, 150, 32, "Reset overrides", TFT_DARKGREY);

  uiDrawSpindleInfo();
  uiDrawNav();
}

void uiDrawSettingsDiagnostics() {
  tft.fillRect(6, 190, SCREEN_W - 12, 88, TFT_BLACK);
  tft.drawRect(6, 190, SCREEN_W - 12, 88, TFT_DARKGREY);

  tft.setTextDatum(ML_DATUM);
  tft.setTextFont(1);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

  char buf[100];
  unsigned long age = state.lastStatusMs ? (millis() - state.lastStatusMs) : 0;
  if (state.linkMode == LINK_UART) {
    snprintf(buf, sizeof(buf), "Modo: UART   Enlace: %s   Ultimo estado: %lu ms   Cola: %s",
             state.linkOk ? "OK" : "SIN LINK", age, cmd.busy ? "ocupada" : "libre");
  } else {
    snprintf(buf, sizeof(buf), "Modo: WiFi   WiFi:%s TCP:%s   Ultimo estado: %lu ms",
             state.wifiConnected ? "OK" : "NO", state.tcpConnected ? "OK" : "NO", age);
  }
  tft.drawString(buf, 12, 202);

  if (state.linkMode == LINK_WIFI) {
    snprintf(buf, sizeof(buf), "DLC32: %s:%d   IP CYD: %s",
             state.dlcIp, state.dlcPort, WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString().c_str() : "-");
    tft.drawString(buf, 12, 214);
  }

  if (state.lastError[0] != 0) {
    snprintf(buf, sizeof(buf), "Ultimo error: %s", state.lastError);
    tft.setTextColor(TFT_RED, TFT_BLACK);
  } else {
    snprintf(buf, sizeof(buf), "Sin errores registrados");
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  }
  int errY = state.linkMode == LINK_WIFI ? 226 : 220;
  tft.drawString(buf, 12, errY);

  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  snprintf(buf, sizeof(buf), "Estado: %s   Feed ov: %d%%   Spindle ov: %d%%   Cola: %s",
           state.grblState, state.feedOv, state.spindleOv, cmd.busy ? "ocupada" : "libre");
  int stY = state.linkMode == LINK_WIFI ? 238 : 238;
  tft.drawString(buf, 12, stY);

  tft.setTextFont(2);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  if (state.message[0] != 0 && millis() - state.messageMs < 6000) {
    tft.drawString(state.message, 12, 260);
  }
}

void uiDrawSettings() {
  uiDrawStatusBar();
  tft.fillRect(0, TOP_H, SCREEN_W, NAV_Y - TOP_H, TFT_BLACK);

  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("AJUSTES", SCREEN_W / 2, 38);

  tft.setTextDatum(ML_DATUM);
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Avance jog (mm/min)", 10, 54);

  char buf[40];
  snprintf(buf, sizeof(buf), "%.0f", state.jogFeed);
  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(buf, 86, 82);

  uiDrawButton(150, 66, 70, 30, "-100", TFT_DARKGREY);
  uiDrawButton(228, 66, 70, 30, "+100", TFT_DARKGREY);
  uiDrawButton(306, 66, 70, 30, "500", TFT_DARKGREY);

  tft.setTextDatum(ML_DATUM);
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("RPM maximo", 10, 112);

  snprintf(buf, sizeof(buf), "%d", state.maxRpm);
  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(4);
  tft.drawString(buf, 86, 126);

  uiDrawButton(150, 110, 70, 30, "-100", TFT_DARKGREY);
  uiDrawButton(228, 110, 70, 30, "+100", TFT_DARKGREY);
  uiDrawButton(306, 110, 70, 30, "1000", TFT_DARKGREY);

  uiDrawButton(10, 146, 150, 28, state.linkMode == LINK_WIFI ? "Modo: WiFi" : "Modo: UART", TFT_NAVY);
  uiDrawButton(170, 146, 150, 28, "WiFi/DLC32", TFT_NAVY);
  uiDrawButton(330, 146, 140, 28, "Recalibrar tactil", TFT_MAROON);

  uiDrawSettingsDiagnostics();
  uiDrawNav();
}

void uiDraw() {
  if (state.page == PAGE_SPINDLE) {
    uiDrawSpindle();
  } else if (state.page == PAGE_SETTINGS) {
    uiDrawSettings();
  } else {
    uiDrawMain();
  }
}

void uiBegin() {
  state.statusDirty = false;
  state.pageDirty = false;
  uiDraw();
}

void uiUpdate() {
  if (state.pageDirty) {
    state.pageDirty = false;
    uiDraw();
  }

  if (state.statusDirty) {
    state.statusDirty = false;
    uiDrawStatusBar();

    if (state.page == PAGE_MAIN) {
      uiDrawDro();
      uiDrawInfo();
    } else if (state.page == PAGE_SPINDLE) {
      uiDrawSpindleInfo();
    } else {
      uiDrawSettingsDiagnostics();
    }
  }
}

static bool uiConfirm(ConfirmAction action, const char *prompt) {
  if (state.confirm == action && millis() - state.confirmMs < 4000) {
    state.confirm = CONFIRM_NONE;
    return true;
  }

  state.confirm = action;
  state.confirmMs = millis();
  grblSetMessage(prompt);
  return false;
}

static void jogAxis(char axis, float direction) {
  if (state.alarm) {
    grblSetError("Alarma activa, usa $X");
    return;
  }
  if (state.homing) {
    grblSetMessage("Homing en curso");
    return;
  }

  char buf[56];
  snprintf(buf, sizeof(buf), "$J=G91 %c%.3f F%.0f",
           axis, state.jogStep * direction, state.jogFeed);
  grblSendCommand(buf, true);
}

static void zeroAxis(char axis) {
  char buf[32];
  snprintf(buf, sizeof(buf), "G10 L20 P0 %c0", axis);
  grblSendCommand(buf);
}

static void zeroAll() {
  grblSendCommand("G10 L20 P0 X0 Y0 Z0");
}

static void spindleSetRpm(int rpm) {
  if (rpm < 0) rpm = 0;
  if (rpm > state.maxRpm) rpm = state.maxRpm;
  state.targetRpm = rpm;
  settingsSave();
  state.pageDirty = true;
  grblSetMessage("Objetivo RPM actualizado");
}

static void spindleOn(SpindleDir dir) {
  char buf[32];
  snprintf(buf, sizeof(buf), "%s S%d",
           dir == SPINDLE_CCW ? "M4" : "M3", state.targetRpm);
  grblSendCommand(buf);
  state.spindleDir = dir;
  state.pageDirty = true;
  grblSetMessage(dir == SPINDLE_CCW ? "Spindle CCW" : "Spindle CW");
}

static void spindleOff() {
  grblSendCommand("M5");
  state.spindleDir = SPINDLE_STOP;
  state.pageDirty = true;
  grblSetMessage("Spindle detenido");
}

static void handleMainTouch(int16_t x, int16_t y) {
  if (y >= STEP_Y && y < STEP_Y + STEP_H && x >= 54 && x < 310) {
    uint8_t idx = (x - 54) / 64;
    if (idx < 4) {
      state.jogStep = stepValues[idx];
      settingsSave();
      uiDrawStepRow();
      grblSetMessage("Paso actualizado");
    }
    return;
  }

  for (uint8_t i = 0; i < 3; i++) {
    int16_t rowY = JOG_Y + i * (JOG_ROW_H + JOG_ROW_GAP);
    if (y >= rowY && y < rowY + JOG_ROW_H) {
      if (x >= 28 && x < 104) {
        jogAxis(axisNames[i][0], -1.0f);
        return;
      }
      if (x >= 108 && x < 184) {
        jogAxis(axisNames[i][0], 1.0f);
        return;
      }
      if (x >= 188 && x < 264) {
        zeroAxis(axisNames[i][0]);
        return;
      }
    }
  }

  if (x >= RIGHT_X && x < RIGHT_X + RIGHT_W) {
    if (y >= 160 && y < 182) {
      if (uiConfirm(CONFIRM_HOME, "Toca Home otra vez para confirmar")) {
        grblSendCommand("$H");
        grblSetMessage("Homing iniciado");
      }
      return;
    }
    if (y >= 184 && y < 206) {
      linkSendRealtime('!');
      grblSetMessage("Feed hold");
      return;
    }
    if (y >= 208 && y < 230) {
      linkSendRealtime('~');
      grblSetMessage("Resume");
      return;
    }
    if (y >= 232 && y < 254) {
      if (uiConfirm(CONFIRM_ZERO_ALL, "Toca Zero All otra vez para confirmar")) {
        zeroAll();
        grblSetMessage("Zero All enviado");
      }
      return;
    }
    if (y >= 256 && y < 278) {
      if (uiConfirm(CONFIRM_ALARM_RESET, "Toca Alarm $X otra vez para confirmar")) {
        grblSendCommand("$X");
        grblSetMessage("Alarm reset enviado");
      }
      return;
    }
  }
}

static void handleSpindleTouch(int16_t x, int16_t y) {
  if (y >= 112 && y < 144) {
    if (x >= 6 && x < 78) {
      spindleSetRpm(state.targetRpm - 50);
    } else if (x >= 84 && x < 156) {
      spindleSetRpm(state.targetRpm + 50);
    } else if (x >= 162 && x < 234) {
      spindleSetRpm(500);
    } else if (x >= 240 && x < 312) {
      spindleSetRpm(1000);
    } else if (x >= 318 && x < 390) {
      if (uiConfirm(CONFIRM_SPINDLE_CW, "Toca M3 CW otra vez para confirmar")) {
        spindleOn(SPINDLE_CW);
      }
    } else if (x >= 396 && x < 468) {
      if (uiConfirm(CONFIRM_SPINDLE_CCW, "Toca M4 CCW otra vez para confirmar")) {
        spindleOn(SPINDLE_CCW);
      }
    }
    return;
  }

  if (y >= 154 && y < 186) {
    if (x >= 6 && x < 78) {
      spindleOff();
    } else if (x >= 84 && x < 156) {
      linkSendRealtime(0x91);
      grblSetMessage("Feed override +");
    } else if (x >= 162 && x < 234) {
      linkSendRealtime(0x92);
      grblSetMessage("Feed override -");
    } else if (x >= 240 && x < 312) {
      linkSendRealtime(0x90);
      grblSetMessage("Feed override 100%");
    } else if (x >= 318 && x < 390) {
      linkSendRealtime(0x9A);
      grblSetMessage("Spindle override +");
    } else if (x >= 396 && x < 468) {
      linkSendRealtime(0x9B);
      grblSetMessage("Spindle override -");
    }
    return;
  }

  if (y >= 196 && y < 228) {
    if (x >= 6 && x < 156) {
      linkSendRealtime(0x99);
      grblSetMessage("Spindle override 100%");
    } else if (x >= 162 && x < 312) {
      linkSendRealtime(0x95);
      grblSetMessage("Rapid override 100%");
    } else if (x >= 318 && x < 468) {
      linkSendRealtime(0x90);
      linkSendRealtime(0x99);
      linkSendRealtime(0x95);
      grblSetMessage("Overrides al 100%");
    }
  }
}

static void handleSettingsTouch(int16_t x, int16_t y) {
  if (y >= 66 && y < 96) {
    if (x >= 150 && x < 220) {
      state.jogFeed -= 100.0f;
      if (state.jogFeed < 50.0f) state.jogFeed = 50.0f;
      settingsSave();
      state.pageDirty = true;
      grblSetMessage("Avance jog actualizado");
    } else if (x >= 228 && x < 298) {
      state.jogFeed += 100.0f;
      if (state.jogFeed > 5000.0f) state.jogFeed = 5000.0f;
      settingsSave();
      state.pageDirty = true;
      grblSetMessage("Avance jog actualizado");
    } else if (x >= 306 && x < 376) {
      state.jogFeed = DEFAULT_JOG_FEED;
      settingsSave();
      state.pageDirty = true;
      grblSetMessage("Avance jog por defecto");
    }
    return;
  }

  if (y >= 110 && y < 140) {
    if (x >= 150 && x < 220) {
      state.maxRpm -= 100;
      if (state.maxRpm < 100) state.maxRpm = 100;
      if (state.targetRpm > state.maxRpm) state.targetRpm = state.maxRpm;
      settingsSave();
      state.pageDirty = true;
      grblSetMessage("RPM maximo actualizado");
    } else if (x >= 228 && x < 298) {
      state.maxRpm += 100;
      if (state.maxRpm > 1000) state.maxRpm = 1000;
      settingsSave();
      state.pageDirty = true;
      grblSetMessage("RPM maximo actualizado");
    } else if (x >= 306 && x < 376) {
      state.maxRpm = DEFAULT_MAX_RPM;
      if (state.targetRpm > state.maxRpm) state.targetRpm = state.maxRpm;
      settingsSave();
      state.pageDirty = true;
      grblSetMessage("RPM maximo por defecto");
    }
    return;
  }

  if (y >= 146 && y < 174) {
    if (x >= 10 && x < 160) {
      if (state.linkMode == LINK_UART) {
        state.linkMode = LINK_WIFI;
        grblSetMessage("Modo: WiFi (reiniciar para aplicar)");
      } else {
        state.linkMode = LINK_UART;
        grblSetMessage("Modo: UART (reiniciar para aplicar)");
      }
      settingsSaveLink();
      state.pageDirty = true;
      return;
    }
    if (x >= 170 && x < 320) {
      if (strcmp(state.dlcIp, "192.168.4.1") == 0) {
        strncpy(state.dlcIp, "192.168.1.100", sizeof(state.dlcIp) - 1);
      } else if (strcmp(state.dlcIp, "192.168.1.100") == 0) {
        strncpy(state.dlcIp, "192.168.1.50", sizeof(state.dlcIp) - 1);
      } else if (strcmp(state.dlcIp, "192.168.1.50") == 0) {
        strncpy(state.dlcIp, "192.168.4.1", sizeof(state.dlcIp) - 1);
      } else {
        strncpy(state.dlcIp, "192.168.4.1", sizeof(state.dlcIp) - 1);
      }
      state.dlcIp[sizeof(state.dlcIp) - 1] = 0;
      if (state.dlcPort == 23) state.dlcPort = 8080;
      else if (state.dlcPort == 8080) state.dlcPort = 23;
      else state.dlcPort = 23;
      settingsSaveLink();
      state.pageDirty = true;
      grblSetMessage("IP/Puerto DLC32 actualizados");
      return;
    }
    if (x >= 330 && x < 470) {
      if (uiConfirm(CONFIRM_TOUCH_CAL, "Toca Recalibrar otra vez para confirmar")) {
        touchCalibrate(true);
        uiBegin();
        grblSetMessage("Calibracion completada");
      }
      return;
    }
  }
}

void uiHandleTouch(int16_t x, int16_t y) {
  if (y >= NAV_Y && y < NAV_Y + NAV_H) {
    uint8_t page = (x < 6) ? 0 : (x - 6) / 158;
    if (page > 2) page = 2;
    if (state.page != (Page)page) {
      state.page = (Page)page;
      state.confirm = CONFIRM_NONE;
      state.pageDirty = true;
    }
    return;
  }

  if (state.page == PAGE_MAIN) {
    handleMainTouch(x, y);
  } else if (state.page == PAGE_SPINDLE) {
    handleSpindleTouch(x, y);
  } else {
    handleSettingsTouch(x, y);
  }
}
