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

#define BOX_PANEL_X 6
#define BOX_PANEL_W 226
#define BOX_PREVIEW_X 238
#define BOX_PREVIEW_Y 48
#define BOX_PREVIEW_W 236
#define BOX_PREVIEW_H 180
#define BOX_ACTION_Y 248
#define BOX_ACTION_H 30

#define BOX_KEY_X0 13
#define BOX_KEY_Y0 118
#define BOX_KEY_W 50
#define BOX_KEY_H 32
#define BOX_KEY_GAP 4

static const float stepValues[4] = {0.1f, 1.0f, 10.0f, 100.0f};
static const char *stepLabels[4] = {"0.1", "1", "10", "100"};
static const char *axisNames[3] = {"X", "Y", "Z"};

enum BoxView : uint8_t {
  BOX_VIEW_DIMENSIONS = 0,
  BOX_VIEW_PARAMETERS
};

static BoxView boxView = BOX_VIEW_DIMENSIONS;
static BoxField editField = BOX_FIELD_NONE;
static bool editActive = false;
static bool editReplace = false;
static char editBuffer[12] = "";

static char lastStatusState[12] = "";
static uint16_t lastStatusColor = 0;
static char lastStatusLink[10] = "";
static uint16_t lastStatusLinkColor = 0;
static char lastDro[3][20] = {"", "", ""};
static char lastInfo[80] = "";
static uint16_t lastInfoColor = 0;
static char lastSpindleInfo[80] = "";
static char lastSpindleNote[80] = "";
static uint16_t lastSpindleNoteColor = 0;
static char lastDiagLine1[100] = "";
static char lastDiagLine2[100] = "";
static char lastDiagLine3[100] = "";
static char lastDiagLine4[100] = "";
static char lastDiagMessage[80] = "";
static uint16_t lastDiagErrorColor = 0;
static char lastJobStatus[80] = "";
static uint16_t lastJobStatusColor = 0;
static char lastAbortReason[80] = "";
static char lastAbortMessage[80] = "";
static uint16_t lastAbortMessageColor = 0;
static JobState lastActionJobState = (JobState)0xFF;

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

static void uiDrawTextIfChanged(char *cache, size_t cacheSize, const char *text,
                                int16_t x, int16_t y, uint16_t color,
                                uint16_t *lastColor, bool force, uint8_t font) {
  bool colorChanged = (lastColor != nullptr) && (*lastColor != color);
  if (force || strcmp(cache, text) != 0 || colorChanged) {
    tft.setTextDatum(ML_DATUM);
    tft.setTextFont(font);
    tft.setTextColor(color, TFT_BLACK);
    tft.setTextPadding(SCREEN_W - 24);
    tft.drawString(text, x, y);
    tft.setTextPadding(0);
    strncpy(cache, text, cacheSize - 1);
    cache[cacheSize - 1] = 0;
    if (lastColor != nullptr) *lastColor = color;
  }
}

static void uiResetCaches() {
  lastStatusState[0] = 0;
  lastStatusLink[0] = 0;
  lastStatusColor = 0;
  lastStatusLinkColor = 0;
  for (uint8_t i = 0; i < 3; i++) lastDro[i][0] = 0;
  lastInfo[0] = 0;
  lastInfoColor = 0;
  lastSpindleInfo[0] = 0;
  lastSpindleNote[0] = 0;
  lastSpindleNoteColor = 0;
  lastDiagLine1[0] = 0;
  lastDiagLine2[0] = 0;
  lastDiagLine3[0] = 0;
  lastDiagLine4[0] = 0;
  lastDiagMessage[0] = 0;
  lastDiagErrorColor = 0;
  lastJobStatus[0] = 0;
  lastJobStatusColor = 0;
  lastAbortReason[0] = 0;
  lastAbortMessage[0] = 0;
  lastAbortMessageColor = 0;
  lastActionJobState = (JobState)0xFF;
  editActive = false;
  editField = BOX_FIELD_NONE;
}

void uiDrawStatusBar(bool force) {
  if (force) {
    tft.fillRect(0, 0, SCREEN_W, TOP_H, TFT_NAVY);
    tft.setTextDatum(ML_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, TFT_NAVY);
    tft.drawString("CYD PENDANT", 6, TOP_H / 2);
    lastStatusState[0] = 0;
    lastStatusLink[0] = 0;
    lastStatusColor = 0;
    lastStatusLinkColor = 0;
  }

  uint16_t stateColor = TFT_WHITE;
  if (state.alarm) stateColor = TFT_RED;
  else if (state.homing) stateColor = TFT_CYAN;

  if (force || strcmp(state.grblState, lastStatusState) != 0 ||
      stateColor != lastStatusColor) {
    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(stateColor, TFT_NAVY);
    tft.setTextPadding(140);
    tft.drawString(state.grblState, SCREEN_W / 2, TOP_H / 2);
    tft.setTextPadding(0);
    strncpy(lastStatusState, state.grblState, sizeof(lastStatusState) - 1);
    lastStatusState[sizeof(lastStatusState) - 1] = 0;
    lastStatusColor = stateColor;
  }

  const char *linkText = state.linkOk ? "LINK" : "SIN LINK";
  uint16_t linkColor = state.linkOk ? TFT_GREEN : TFT_RED;
  if (force || strcmp(linkText, lastStatusLink) != 0 ||
      linkColor != lastStatusLinkColor) {
    tft.setTextDatum(MR_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(linkColor, TFT_NAVY);
    tft.setTextPadding(90);
    tft.drawString(linkText, SCREEN_W - 6, TOP_H / 2);
    tft.setTextPadding(0);
    strncpy(lastStatusLink, linkText, sizeof(lastStatusLink) - 1);
    lastStatusLink[sizeof(lastStatusLink) - 1] = 0;
    lastStatusLinkColor = linkColor;
  }
}

void uiDrawDro(bool force) {
  const float *pos = state.wposValid ? state.wpos : state.mpos;

  if (force) {
    tft.fillRect(DRO_X, DRO_Y, DRO_W, DRO_H, TFT_BLACK);
    tft.drawRect(DRO_X, DRO_Y, DRO_W, DRO_H, TFT_DARKGREY);
    for (uint8_t i = 0; i < 3; i++) {
      int16_t cellX = DRO_X + i * 156;
      if (i > 0) tft.drawFastVLine(cellX, DRO_Y + 4, DRO_H - 8, TFT_DARKGREY);

      tft.setTextDatum(TC_DATUM);
      tft.setTextFont(2);
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.drawString(axisNames[i], cellX + 78, DRO_Y + 5);
      lastDro[i][0] = 0;
    }
  }

  for (uint8_t i = 0; i < 3; i++) {
    int16_t cellX = DRO_X + i * 156;
    char buf[20];
    snprintf(buf, sizeof(buf), "%.2f", pos[i]);
    if (force || strcmp(buf, lastDro[i]) != 0) {
      tft.setTextDatum(TC_DATUM);
      tft.setTextFont(4);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setTextPadding(148);
      tft.drawString(buf, cellX + 78, DRO_Y + 30);
      tft.setTextPadding(0);
      strncpy(lastDro[i], buf, sizeof(lastDro[i]) - 1);
      lastDro[i][sizeof(lastDro[i]) - 1] = 0;
    }
  }
}

void uiDrawInfo(bool force) {
  if (force) {
    tft.fillRect(6, INFO_Y, SCREEN_W - 12, 16, TFT_BLACK);
    lastInfo[0] = 0;
    lastInfoColor = 0;
  }

  char buf[80];
  uint16_t color;
  if (state.message[0] != 0 && millis() - state.messageMs < 4000) {
    snprintf(buf, sizeof(buf), "%s", state.message);
    color = TFT_YELLOW;
  } else {
    snprintf(buf, sizeof(buf), "F %.0f  S %.0f  Ov %d/%d/%d  %s",
             state.feed, state.spindle, state.feedOv, state.rapidOv,
             state.spindleOv, state.wposValid ? "WPos" : "MPos");
    color = TFT_LIGHTGREY;
  }

  if (force || strcmp(buf, lastInfo) != 0 || color != lastInfoColor) {
    tft.setTextDatum(ML_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(color, TFT_BLACK);
    tft.setTextPadding(SCREEN_W - 16);
    tft.drawString(buf, 8, INFO_Y + 8);
    tft.setTextPadding(0);
    strncpy(lastInfo, buf, sizeof(lastInfo) - 1);
    lastInfo[sizeof(lastInfo) - 1] = 0;
    lastInfoColor = color;
  }
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
  static const char *labels[4] = {"Principal", "Spindle", "Caja", "Ajustes"};

  for (uint8_t i = 0; i < 4; i++) {
    int16_t x = 6 + i * 118;
    bool selected = (state.page == (Page)i);
    uiDrawButton(x, NAV_Y, 114, NAV_H, labels[i],
                 selected ? TFT_BLUE : TFT_DARKGREY, selected);
  }
}

void uiDrawMain() {
  uiDrawStatusBar(true);
  tft.fillRect(0, TOP_H, SCREEN_W, NAV_Y - TOP_H, TFT_BLACK);
  uiDrawDro(true);
  uiDrawInfo(true);
  uiDrawStepRow();
  uiDrawJog();
  uiDrawNav();
}

void uiDrawSpindleInfo(bool force) {
  if (force) {
    tft.fillRect(6, 238, SCREEN_W - 12, 40, TFT_BLACK);
    lastSpindleInfo[0] = 0;
    lastSpindleNote[0] = 0;
    lastSpindleNoteColor = 0;
  }

  char buf[80];
  snprintf(buf, sizeof(buf), "Actual  F %.0f  S %.0f   Objetivo %d RPM",
           state.feed, state.spindle, state.targetRpm);

  if (force || strcmp(buf, lastSpindleInfo) != 0) {
    tft.setTextDatum(ML_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setTextPadding(SCREEN_W - 16);
    tft.drawString(buf, 8, 248);
    tft.setTextPadding(0);
    strncpy(lastSpindleInfo, buf, sizeof(lastSpindleInfo) - 1);
    lastSpindleInfo[sizeof(lastSpindleInfo) - 1] = 0;
  }

  const char *note;
  uint16_t noteColor;
  if (state.message[0] != 0 && millis() - state.messageMs < 4000) {
    note = state.message;
    noteColor = TFT_YELLOW;
  } else {
    note = "M3/M4 aplican objetivo. M4 requiere $32=0 y DIR.";
    noteColor = TFT_DARKGREY;
  }
  uiDrawTextIfChanged(lastSpindleNote, sizeof(lastSpindleNote), note, 8, 266,
                      noteColor, &lastSpindleNoteColor, force, 1);
}

void uiDrawSpindle() {
  uiDrawStatusBar(true);
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

  uiDrawSpindleInfo(true);
  uiDrawNav();
}

void uiDrawSettingsDiagnostics(bool force) {
  if (force) {
    tft.fillRect(6, 190, SCREEN_W - 12, 88, TFT_BLACK);
    tft.drawRect(6, 190, SCREEN_W - 12, 88, TFT_DARKGREY);
    lastDiagLine1[0] = 0;
    lastDiagLine2[0] = 0;
    lastDiagLine3[0] = 0;
    lastDiagLine4[0] = 0;
    lastDiagMessage[0] = 0;
    lastDiagErrorColor = 0;
  }

  char buf[100];
  unsigned long age = state.lastStatusMs ? (millis() - state.lastStatusMs) : 0;
  unsigned long ageSec = age / 1000UL;

  if (state.linkMode == LINK_UART) {
    snprintf(buf, sizeof(buf), "Modo: UART   Enlace: %s   Ultimo estado: %lu s   Cola: %s",
             state.linkOk ? "OK" : "SIN LINK", ageSec, cmd.busy ? "ocupada" : "libre");
  } else {
    snprintf(buf, sizeof(buf), "Modo: WiFi   WiFi:%s TCP:%s   Ultimo estado: %lu s",
             state.wifiConnected ? "OK" : "NO", state.tcpConnected ? "OK" : "NO", ageSec);
  }
  uiDrawTextIfChanged(lastDiagLine1, sizeof(lastDiagLine1), buf, 12, 202,
                      TFT_LIGHTGREY, nullptr, force, 1);

  if (state.linkMode == LINK_WIFI) {
    snprintf(buf, sizeof(buf), "DLC32: %s:%d   IP CYD: %s",
             state.dlcIp, state.dlcPort,
             WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString().c_str() : "-");
    uiDrawTextIfChanged(lastDiagLine2, sizeof(lastDiagLine2), buf, 12, 214,
                        TFT_LIGHTGREY, nullptr, force, 1);
  }

  uint16_t errorColor = state.lastError[0] != 0 ? TFT_RED : TFT_DARKGREY;
  if (state.lastError[0] != 0) {
    snprintf(buf, sizeof(buf), "Ultimo error: %s", state.lastError);
  } else {
    snprintf(buf, sizeof(buf), "Sin errores registrados");
  }
  int errY = state.linkMode == LINK_WIFI ? 226 : 220;
  uiDrawTextIfChanged(lastDiagLine3, sizeof(lastDiagLine3), buf, 12, errY,
                      errorColor, &lastDiagErrorColor, force, 1);

  snprintf(buf, sizeof(buf), "Estado: %s   Feed ov: %d%%   Spindle ov: %d%%   Cola: %s",
           state.grblState, state.feedOv, state.spindleOv, cmd.busy ? "ocupada" : "libre");
  uiDrawTextIfChanged(lastDiagLine4, sizeof(lastDiagLine4), buf, 12, 238,
                      TFT_LIGHTGREY, nullptr, force, 1);

  if (state.message[0] != 0 && millis() - state.messageMs < 6000) {
    snprintf(buf, sizeof(buf), "%s", state.message);
  } else {
    buf[0] = 0;
  }
  uiDrawTextIfChanged(lastDiagMessage, sizeof(lastDiagMessage), buf, 12, 260,
                      TFT_YELLOW, nullptr, force, 2);
}

void uiDrawSettings() {
  uiDrawStatusBar(true);
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

  uiDrawSettingsDiagnostics(true);
  uiDrawNav();
}

static void uiDrawDashedLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                             uint16_t color, uint8_t dash) {
  int16_t dx = x2 - x1;
  int16_t dy = y2 - y1;
  int16_t steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
  if (steps == 0) return;

  for (int16_t i = 0; i <= steps; i += dash * 2) {
    int16_t end = i + dash;
    if (end > steps) end = steps;
    int16_t xa = x1 + (int32_t)dx * i / steps;
    int16_t ya = y1 + (int32_t)dy * i / steps;
    int16_t xb = x1 + (int32_t)dx * end / steps;
    int16_t yb = y1 + (int32_t)dy * end / steps;
    tft.drawLine(xa, ya, xb, yb, color);
  }
}

static void uiDrawPrism() {
  tft.fillRect(BOX_PREVIEW_X, BOX_PREVIEW_Y, BOX_PREVIEW_W, BOX_PREVIEW_H, TFT_BLACK);
  tft.drawRect(BOX_PREVIEW_X, BOX_PREVIEW_Y, BOX_PREVIEW_W, BOX_PREVIEW_H, TFT_DARKGREY);

  tft.setTextDatum(TL_DATUM);
  tft.setTextFont(1);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("VISTA 3D", BOX_PREVIEW_X + 8, BOX_PREVIEW_Y + 6);

  float L = state.boxLength;
  float W = state.boxWidth;
  float H = state.boxHeight;
  if (L < 0.1f) L = 0.1f;
  if (W < 0.1f) W = 0.1f;
  if (H < 0.1f) H = 0.1f;

  float availW = BOX_PREVIEW_W - 42.0f;
  float availH = BOX_PREVIEW_H - 42.0f;
  float depthX = W * 0.55f;
  float depthY = W * 0.38f;
  float totalW = L + depthX;
  float totalH = H + depthY;
  float scale = fminf(availW / totalW, availH / totalH);
  if (scale <= 0.0f) return;

  float fw = L * scale;
  float fh = H * scale;
  float dx = depthX * scale;
  float dy = depthY * scale;

  float left = BOX_PREVIEW_X + 30.0f + (availW - (fw + dx)) * 0.5f;
  float top = BOX_PREVIEW_Y + 22.0f + (availH - (fh + dy)) * 0.5f;
  float bottom = top + fh + dy;

  int16_t x0 = (int16_t)roundf(left);
  int16_t y0 = (int16_t)roundf(bottom);
  int16_t x1 = (int16_t)roundf(left + fw);
  int16_t y1 = (int16_t)roundf(bottom - fh);
  int16_t x2 = (int16_t)roundf(left + dx);
  int16_t y2 = (int16_t)roundf(bottom - dy);
  int16_t x3 = (int16_t)roundf(left + fw + dx);
  int16_t y3 = (int16_t)roundf(bottom - fh - dy);
  int16_t x4 = (int16_t)roundf(left + fw + dx);
  int16_t y4 = (int16_t)roundf(bottom - dy);

  int16_t rw = (int16_t)roundf(fw);
  int16_t rh = (int16_t)roundf(fh);
  if (rw < 1) rw = 1;
  if (rh < 1) rh = 1;

  tft.drawRect(x0, y1, rw, rh, TFT_CYAN);
  tft.drawLine(x0, y1, x2, y3, TFT_BLUE);
  tft.drawLine(x2, y3, x3, y3, TFT_BLUE);
  tft.drawLine(x3, y3, x1, y1, TFT_BLUE);
  tft.drawLine(x1, y1, x0, y1, TFT_BLUE);

  tft.drawLine(x1, y1, x3, y3, TFT_DARKCYAN);
  tft.drawLine(x3, y3, x4, y4, TFT_DARKCYAN);
  tft.drawLine(x4, y4, x1, y0, TFT_DARKCYAN);
  tft.drawLine(x1, y0, x1, y1, TFT_DARKCYAN);

  uiDrawDashedLine(x2, y3, x2, y2, TFT_DARKGREY, 4);
  uiDrawDashedLine(x2, y2, x4, y2, TFT_DARKGREY, 4);
  uiDrawDashedLine(x0, y0, x2, y2, TFT_DARKGREY, 4);

  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("CAVIDAD",
                 (x0 + x1 + x2 + x3) / 4,
                 (y1 + y1 + y3 + y3) / 4);

  char buf[24];
  snprintf(buf, sizeof(buf), "L %.1f", L);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString(buf, x0 + rw / 2, y0 + 10);

  snprintf(buf, sizeof(buf), "H%.1f", H);
  tft.setTextDatum(MR_DATUM);
  tft.drawString(buf, x0 - 5, y1 + rh / 2);

  snprintf(buf, sizeof(buf), "W %.1f", W);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(buf, x1 + (int16_t)(dx / 2.0f), y1 - (int16_t)(dy / 2.0f) - 6);
}

static void uiDrawBoxValueRow(int16_t y, const char *label, float value,
                              uint8_t decimals) {
  tft.drawRect(6, y, BOX_PANEL_W, 48, TFT_DARKGREY);

  tft.setTextDatum(ML_DATUM);
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(label, 12, y + 12);

  char buf[24];
  snprintf(buf, sizeof(buf), "%.*f", decimals, value);
  tft.setTextDatum(MR_DATUM);
  tft.setTextFont(4);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextPadding(120);
  tft.drawString(buf, 224, y + 34);
  tft.setTextPadding(0);
}

static void uiDrawBoxDimensions() {
  uiDrawBoxValueRow(50, "Largo X (mm)", state.boxLength, 1);
  uiDrawBoxValueRow(102, "Ancho Y (mm)", state.boxWidth, 1);
  uiDrawBoxValueRow(154, "Altura Z (mm)", state.boxHeight, 1);

  tft.setTextDatum(ML_DATUM);
  tft.setTextFont(1);
  tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft.drawString("Toca un valor para editarlo", 10, 216);
}

static void uiDrawBoxParameters() {
  static const char *labels[7] = {
    "Diametro fresa (mm)",
    "Paso lateral (%)",
    "Avance F (mm/min)",
    "RPM husillo",
    "Z seguro (mm)",
    "Bajada por pasada (mm)",
    "Avance penetracion (mm/min)"
  };

  char values[7][24];
  snprintf(values[0], sizeof(values[0]), "%.1f", state.boxToolDia);
  snprintf(values[1], sizeof(values[1]), "%.0f", state.boxStepoverPct);
  snprintf(values[2], sizeof(values[2]), "%.0f", state.boxFeed);
  snprintf(values[3], sizeof(values[3]), "%d", state.boxRpm);
  snprintf(values[4], sizeof(values[4]), "%.1f", state.boxSafeZ);
  snprintf(values[5], sizeof(values[5]), "%.2f", state.boxDepthStep);
  snprintf(values[6], sizeof(values[6]), "%.0f", state.boxPlungeFeed);

  for (uint8_t i = 0; i < 7; i++) {
    int16_t y = 50 + i * 26;
    tft.drawFastHLine(6, y + 24, BOX_PANEL_W, TFT_DARKGREY);

    tft.setTextDatum(ML_DATUM);
    tft.setTextFont(1);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString(labels[i], 10, y + 12);

    tft.setTextDatum(MR_DATUM);
    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextPadding(120);
    tft.drawString(values[i], 224, y + 12);
    tft.setTextPadding(0);
  }
}

static const char *boxFieldLabel(BoxField field) {
  switch (field) {
    case BOX_FIELD_LENGTH: return "Largo X (mm)";
    case BOX_FIELD_WIDTH: return "Ancho Y (mm)";
    case BOX_FIELD_HEIGHT: return "Altura Z (mm)";
    case BOX_FIELD_TOOL: return "Diametro fresa (mm)";
    case BOX_FIELD_STEPOVER: return "Paso lateral (%)";
    case BOX_FIELD_FEED: return "Avance F (mm/min)";
    case BOX_FIELD_RPM: return "RPM husillo";
    case BOX_FIELD_SAFE_Z: return "Z seguro (mm)";
    case BOX_FIELD_DEPTH_STEP: return "Bajada por pasada (mm)";
    case BOX_FIELD_PLUNGE: return "Avance penetracion (mm/min)";
    default: return "";
  }
}

static void uiDrawBoxKeypadValue() {
  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextPadding(200);
  tft.drawString(editBuffer[0] ? editBuffer : "0", 119, 92);
  tft.setTextPadding(0);
}

static void uiDrawBoxKeypadMessage() {
  char buf[64];
  if (state.message[0] != 0) {
    snprintf(buf, sizeof(buf), "%s", state.message);
  } else {
    buf[0] = 0;
  }
  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextPadding(210);
  tft.drawString(buf, 119, 270);
  tft.setTextPadding(0);
}

static void uiDrawBoxKeypad() {
  tft.fillRect(6, 48, BOX_PANEL_W, 232, TFT_BLACK);
  tft.drawRect(6, 48, BOX_PANEL_W, 232, TFT_WHITE);
  tft.fillRect(6 + BOX_PANEL_W + 1, 234,
               SCREEN_W - (6 + BOX_PANEL_W + 1), 46, TFT_BLACK);

  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(1);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString(boxFieldLabel(editField), 119, 62);

  uiDrawBoxKeypadValue();

  static const char *row0[4] = {"1", "2", "3", "Del"};
  static const char *row1[4] = {"4", "5", "6", "C"};
  static const char *row2[4] = {"7", "8", "9", "."};

  for (uint8_t col = 0; col < 4; col++) {
    int16_t x = BOX_KEY_X0 + col * (BOX_KEY_W + BOX_KEY_GAP);
    uint16_t color = (col == 3) ? TFT_MAROON : TFT_DARKGREY;
    uiDrawButton(x, BOX_KEY_Y0, BOX_KEY_W, BOX_KEY_H, row0[col], color, false, 2);
    uiDrawButton(x, BOX_KEY_Y0 + 36, BOX_KEY_W, BOX_KEY_H, row1[col], color, false, 2);
    uiDrawButton(x, BOX_KEY_Y0 + 72, BOX_KEY_W, BOX_KEY_H, row2[col], color, false, 2);
  }

  uiDrawButton(BOX_KEY_X0, BOX_KEY_Y0 + 108, BOX_KEY_W, BOX_KEY_H,
               "0", TFT_DARKGREY, false, 2);
  uiDrawButton(BOX_KEY_X0 + 54, BOX_KEY_Y0 + 108, BOX_KEY_W, BOX_KEY_H,
               "Cancel", TFT_DARKGREY, false, 2);
  uiDrawButton(BOX_KEY_X0 + 108, BOX_KEY_Y0 + 108, BOX_KEY_W * 2 + BOX_KEY_GAP,
               BOX_KEY_H, "OK", TFT_DARKGREEN, false, 2);

  uiDrawBoxKeypadMessage();
}

static void uiDrawBoxActions() {
  const char *toggleLabel = (boxView == BOX_VIEW_DIMENSIONS) ? "Parametros" : "Dimensiones";
  uiDrawButton(6, BOX_ACTION_Y, 100, BOX_ACTION_H, toggleLabel, TFT_DARKGREY, false, 2);
  uiDrawButton(112, BOX_ACTION_Y, 100, BOX_ACTION_H, "Generar", TFT_NAVY, false, 2);

  const char *runLabel = "Ejecutar";
  uint16_t runColor = TFT_DARKGREEN;
  if (state.jobState == JOB_RUNNING) {
    runLabel = "Pausa";
    runColor = TFT_ORANGE;
  } else if (state.jobState == JOB_PAUSED) {
    runLabel = "Reanudar";
    runColor = TFT_DARKGREEN;
  }
  uiDrawButton(218, BOX_ACTION_Y, 120, BOX_ACTION_H, runLabel, runColor, false, 2);
  uiDrawButton(344, BOX_ACTION_Y, 130, BOX_ACTION_H, "Detener", TFT_MAROON, false, 2);

  lastActionJobState = state.jobState;
}

static void uiDrawBoxJobStatus(bool force) {
  if (lastActionJobState != state.jobState) uiDrawBoxActions();

  if (force) {
    tft.fillRect(6, 234, SCREEN_W - 12, 12, TFT_BLACK);
    lastJobStatus[0] = 0;
    lastJobStatusColor = 0;
  }

  char buf[80];
  uint16_t color = TFT_DARKGREY;

  if (state.message[0] != 0 && millis() - state.messageMs < 4000) {
    snprintf(buf, sizeof(buf), "%s", state.message);
    color = TFT_YELLOW;
  } else if (state.jobState == JOB_RUNNING) {
    uint16_t pct = state.jobTotalLines ?
                   (uint16_t)((uint32_t)state.jobLine * 100UL / state.jobTotalLines) : 0;
    snprintf(buf, sizeof(buf), "Ejecutando %u/%u lineas (%u%%)",
             (unsigned)state.jobLine, (unsigned)state.jobTotalLines, (unsigned)pct);
    color = TFT_CYAN;
  } else if (state.jobState == JOB_PAUSED) {
    uint16_t pct = state.jobTotalLines ?
                   (uint16_t)((uint32_t)state.jobLine * 100UL / state.jobTotalLines) : 0;
    snprintf(buf, sizeof(buf), "Pausa %u/%u lineas (%u%%)",
             (unsigned)state.jobLine, (unsigned)state.jobTotalLines, (unsigned)pct);
    color = TFT_ORANGE;
  } else if (state.jobState == JOB_ERROR) {
    snprintf(buf, sizeof(buf), "ERROR: %s",
             state.lastError[0] ? state.lastError : "trabajo detenido");
    color = TFT_RED;
  } else {
    snprintf(buf, sizeof(buf), "Listo. Ajusta, genera y ejecuta.");
  }

  uiDrawTextIfChanged(lastJobStatus, sizeof(lastJobStatus), buf, 8, 240,
                      color, &lastJobStatusColor, force, 1);
}

void uiDrawBox() {
  uiDrawStatusBar(true);
  tft.fillRect(0, TOP_H, SCREEN_W, NAV_Y - TOP_H, TFT_BLACK);

  tft.setTextDatum(MC_DATUM);
  tft.setTextFont(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("CAJA / DESBASTE", SCREEN_W / 2, 38);

  uiDrawPrism();

  if (boxView == BOX_VIEW_DIMENSIONS) {
    uiDrawBoxDimensions();
  } else {
    uiDrawBoxParameters();
  }

  uiDrawBoxActions();
  uiDrawBoxJobStatus(true);
  uiDrawNav();

  if (editActive) uiDrawBoxKeypad();
}

static void uiDrawJobAbortPrompt(bool force) {
  if (force) {
    tft.fillRect(0, TOP_H, SCREEN_W, NAV_Y + NAV_H - TOP_H, TFT_BLACK);
    tft.drawRect(6, 34, SCREEN_W - 12, 272, TFT_RED);

    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(4);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawString("TRABAJO DETENIDO", SCREEN_W / 2, 54);

    tft.setTextFont(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("HUSILLO PUEDE SEGUIR GIRANDO", SCREEN_W / 2, 118);

    tft.setTextDatum(ML_DATUM);
    tft.setTextFont(1);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.drawString("M5 intenta parar. RESET garantiza la parada.", 16, 142);
    tft.drawString("LISTO cierra despues de verificar la maquina.", 16, 156);

    lastAbortReason[0] = 0;
    lastAbortMessage[0] = 0;
    lastAbortMessageColor = 0;

    uiDrawButton(12, 236, 140, 32, "M5 HUSILLO", TFT_MAROON, false, 2);
    uiDrawButton(164, 236, 152, 32, "RESET GRBL", TFT_DARKGREY, false, 2);
    uiDrawButton(328, 236, 140, 32, "LISTO", TFT_DARKGREEN, false, 2);
  }

  const char *reason = state.lastError[0] ? state.lastError : "Trabajo detenido";
  uiDrawTextIfChanged(lastAbortReason, sizeof(lastAbortReason), reason, 16, 90,
                      TFT_RED, nullptr, force, 2);

  const char *message = state.message[0] ? state.message : "";
  uiDrawTextIfChanged(lastAbortMessage, sizeof(lastAbortMessage), message, 16, 184,
                      TFT_YELLOW, &lastAbortMessageColor, force, 2);

  char buf[48];
  snprintf(buf, sizeof(buf), "Estado GRBL: %s   Cola: %s",
           state.grblState, cmd.busy ? "ocupada" : "libre");
  tft.setTextDatum(ML_DATUM);
  tft.setTextFont(1);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextPadding(SCREEN_W - 32);
  tft.drawString(buf, 16, 210);
  tft.setTextPadding(0);
}

void uiDraw() {
  if (state.page == PAGE_SPINDLE) {
    uiDrawSpindle();
  } else if (state.page == PAGE_BOX) {
    uiDrawBox();
  } else if (state.page == PAGE_SETTINGS) {
    uiDrawSettings();
  } else {
    uiDrawMain();
  }

  if (state.jobAbortPrompt) uiDrawJobAbortPrompt(true);
}

void uiBegin() {
  uiResetCaches();
  state.statusDirty = false;
  state.pageDirty = false;
  state.jobDirty = false;
  uiDraw();
}

static void uiUpdateStatus() {
  uiDrawStatusBar(false);

  if (state.page == PAGE_MAIN) {
    uiDrawDro(false);
    uiDrawInfo(false);
  } else if (state.page == PAGE_SPINDLE) {
    uiDrawSpindleInfo(false);
  } else if (state.page == PAGE_BOX) {
    if (!editActive) uiDrawBoxJobStatus(false);
  } else {
    uiDrawSettingsDiagnostics(false);
  }
}

void uiUpdate() {
  if (state.pageDirty) {
    state.pageDirty = false;
    uiDraw();
  }

  if (state.jobAbortPrompt) {
    if (state.statusDirty) {
      state.statusDirty = false;
      uiDrawJobAbortPrompt(false);
    }
    state.jobDirty = false;
    return;
  }

  if (state.statusDirty) {
    state.statusDirty = false;
    uiUpdateStatus();
  }

  if (state.jobDirty) {
    state.jobDirty = false;
    if (state.page == PAGE_BOX && !editActive) uiDrawBoxJobStatus(false);
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
  if (!grblSendCommand(buf)) return;

  cmd.spindlePending = true;
  cmd.pendingSpindleDir = dir;
  grblSetMessage(dir == SPINDLE_CCW ? "M4 CCW enviado" : "M3 CW enviado");
}

static void spindleOff() {
  if (!grblSendCommand("M5")) return;

  cmd.spindlePending = true;
  cmd.pendingSpindleDir = SPINDLE_STOP;
  grblSetMessage("M5 enviado");
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
      if (state.boxRpm > state.maxRpm) state.boxRpm = state.maxRpm;
      state.jobTotalLines = 0;
      state.jobDirty = true;
      settingsSave();
      state.pageDirty = true;
      grblSetMessage("RPM maximo actualizado");
    } else if (x >= 228 && x < 298) {
      state.maxRpm += 100;
      if (state.maxRpm > 1000) state.maxRpm = 1000;
      if (state.targetRpm > state.maxRpm) state.targetRpm = state.maxRpm;
      if (state.boxRpm > state.maxRpm) state.boxRpm = state.maxRpm;
      state.jobTotalLines = 0;
      state.jobDirty = true;
      settingsSave();
      state.pageDirty = true;
      grblSetMessage("RPM maximo actualizado");
    } else if (x >= 306 && x < 376) {
      state.maxRpm = DEFAULT_MAX_RPM;
      if (state.targetRpm > state.maxRpm) state.targetRpm = state.maxRpm;
      if (state.boxRpm > state.maxRpm) state.boxRpm = state.maxRpm;
      state.jobTotalLines = 0;
      state.jobDirty = true;
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
      struct LinkPreset { const char *ip; uint16_t port; };
      static const LinkPreset presets[] = {
        {DLC32_DEFAULT_IP, DLC32_DEFAULT_PORT},
        {DLC32_DEFAULT_IP, DLC32_ALT_PORT},
        {"192.168.1.100", DLC32_DEFAULT_PORT},
        {"192.168.1.100", DLC32_ALT_PORT},
        {"192.168.1.50", DLC32_DEFAULT_PORT},
        {"192.168.1.50", DLC32_ALT_PORT},
      };
      const size_t count = sizeof(presets) / sizeof(presets[0]);
      size_t next = 0;

      for (size_t i = 0; i < count; i++) {
        if (strcmp(state.dlcIp, presets[i].ip) == 0 &&
            state.dlcPort == presets[i].port) {
          next = (i + 1) % count;
          break;
        }
      }

      strncpy(state.dlcIp, presets[next].ip, sizeof(state.dlcIp) - 1);
      state.dlcIp[sizeof(state.dlcIp) - 1] = 0;
      state.dlcPort = presets[next].port;
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

static void uiOpenBoxField(BoxField field) {
  if (state.jobState == JOB_RUNNING || state.jobState == JOB_PAUSED) {
    grblSetMessage("Deten el trabajo para editar");
    return;
  }

  editField = field;
  editActive = true;
  editReplace = true;
  state.message[0] = 0;
  state.messageMs = millis();

  switch (field) {
    case BOX_FIELD_LENGTH:
      snprintf(editBuffer, sizeof(editBuffer), "%.1f", state.boxLength);
      break;
    case BOX_FIELD_WIDTH:
      snprintf(editBuffer, sizeof(editBuffer), "%.1f", state.boxWidth);
      break;
    case BOX_FIELD_HEIGHT:
      snprintf(editBuffer, sizeof(editBuffer), "%.1f", state.boxHeight);
      break;
    case BOX_FIELD_TOOL:
      snprintf(editBuffer, sizeof(editBuffer), "%.1f", state.boxToolDia);
      break;
    case BOX_FIELD_STEPOVER:
      snprintf(editBuffer, sizeof(editBuffer), "%.0f", state.boxStepoverPct);
      break;
    case BOX_FIELD_FEED:
      snprintf(editBuffer, sizeof(editBuffer), "%.0f", state.boxFeed);
      break;
    case BOX_FIELD_RPM:
      snprintf(editBuffer, sizeof(editBuffer), "%d", state.boxRpm);
      break;
    case BOX_FIELD_SAFE_Z:
      snprintf(editBuffer, sizeof(editBuffer), "%.1f", state.boxSafeZ);
      break;
    case BOX_FIELD_DEPTH_STEP:
      snprintf(editBuffer, sizeof(editBuffer), "%.2f", state.boxDepthStep);
      break;
    case BOX_FIELD_PLUNGE:
      snprintf(editBuffer, sizeof(editBuffer), "%.0f", state.boxPlungeFeed);
      break;
    default:
      break;
  }

  uiDrawBoxKeypad();
}

static void uiCommitBoxField() {
  char *end = nullptr;
  float value = strtof(editBuffer, &end);
  if (end == editBuffer || *end != 0 || !isfinite(value)) {
    grblSetMessage("Valor invalido");
    uiDrawBoxKeypadMessage();
    return;
  }

  bool ok = true;
  float minDim = 0.0f;

  switch (editField) {
    case BOX_FIELD_LENGTH:
      if (value < 5.0f || value > 300.0f ||
          state.boxToolDia >= fminf(value, state.boxWidth)) {
        ok = false;
      } else {
        state.boxLength = value;
      }
      break;
    case BOX_FIELD_WIDTH:
      if (value < 5.0f || value > 300.0f ||
          state.boxToolDia >= fminf(value, state.boxLength)) {
        ok = false;
      } else {
        state.boxWidth = value;
      }
      break;
    case BOX_FIELD_HEIGHT:
      if (value < 1.0f || value > 100.0f) {
        ok = false;
      } else {
        state.boxHeight = value;
        if (state.boxDepthStep > state.boxHeight) state.boxDepthStep = state.boxHeight;
      }
      break;
    case BOX_FIELD_TOOL:
      minDim = fminf(state.boxLength, state.boxWidth);
      if (value < 0.5f || value > 20.0f || value >= minDim) {
        ok = false;
      } else {
        state.boxToolDia = value;
      }
      break;
    case BOX_FIELD_STEPOVER:
      if (value < 5.0f || value > 80.0f) {
        ok = false;
      } else {
        state.boxStepoverPct = value;
      }
      break;
    case BOX_FIELD_FEED:
      if (value < 50.0f || value > 3000.0f) {
        ok = false;
      } else {
        state.boxFeed = value;
      }
      break;
    case BOX_FIELD_RPM:
      if (value < 100.0f || value > (float)state.maxRpm) {
        ok = false;
      } else {
        state.boxRpm = (int)(value + 0.5f);
      }
      break;
    case BOX_FIELD_SAFE_Z:
      if (value < 1.0f || value > 50.0f) {
        ok = false;
      } else {
        state.boxSafeZ = value;
      }
      break;
    case BOX_FIELD_DEPTH_STEP:
      if (value < 0.1f || value > 10.0f || value > state.boxHeight) {
        ok = false;
      } else {
        state.boxDepthStep = value;
      }
      break;
    case BOX_FIELD_PLUNGE:
      if (value < 10.0f || value > 1000.0f) {
        ok = false;
      } else {
        state.boxPlungeFeed = value;
      }
      break;
    default:
      ok = false;
      break;
  }

  if (!ok) {
    grblSetMessage("Valor fuera de rango");
    uiDrawBoxKeypadMessage();
    return;
  }

  settingsSave();
  state.jobTotalLines = 0;
  state.jobLine = 0;
  if (state.jobState != JOB_RUNNING && state.jobState != JOB_PAUSED) {
    state.jobState = JOB_IDLE;
  }
  state.jobDirty = true;
  state.pageDirty = true;
  editActive = false;
  editField = BOX_FIELD_NONE;
  grblSetMessage("Valor actualizado");
}

static void uiHandleKeypadTouch(int16_t x, int16_t y) {
  int16_t col = (x - BOX_KEY_X0) / (BOX_KEY_W + BOX_KEY_GAP);
  int16_t row = (y - BOX_KEY_Y0) / (BOX_KEY_H + BOX_KEY_GAP);
  if (col < 0 || col > 3 || row < 0 || row > 3) return;

  int16_t cellX = BOX_KEY_X0 + col * (BOX_KEY_W + BOX_KEY_GAP);
  int16_t cellY = BOX_KEY_Y0 + row * (BOX_KEY_H + BOX_KEY_GAP);
  if (x > cellX + BOX_KEY_W || y > cellY + BOX_KEY_H) return;

  const char *key = nullptr;
  if (row == 0) {
    static const char *keys[4] = {"1", "2", "3", "Del"};
    key = keys[col];
  } else if (row == 1) {
    static const char *keys[4] = {"4", "5", "6", "C"};
    key = keys[col];
  } else if (row == 2) {
    static const char *keys[4] = {"7", "8", "9", "."};
    key = keys[col];
  } else if (col == 0) {
    key = "0";
  } else if (col == 1) {
    key = "Cancel";
  } else {
    key = "OK";
  }

  if (strcmp(key, "Cancel") == 0) {
    editActive = false;
    editField = BOX_FIELD_NONE;
    state.pageDirty = true;
    return;
  }

  if (strcmp(key, "OK") == 0) {
    uiCommitBoxField();
    return;
  }

  if (state.message[0] != 0) {
    state.message[0] = 0;
    uiDrawBoxKeypadMessage();
  }

  if (strcmp(key, "C") == 0) {
    editReplace = false;
    editBuffer[0] = 0;
    uiDrawBoxKeypadValue();
    return;
  }

  if (strcmp(key, "Del") == 0) {
    editReplace = false;
    size_t len = strlen(editBuffer);
    if (len > 0) editBuffer[len - 1] = 0;
    uiDrawBoxKeypadValue();
    return;
  }

  if (editReplace) {
    editBuffer[0] = 0;
    editReplace = false;
  }

  if (key[0] == '.') {
    if (strchr(editBuffer, '.') != nullptr) return;
  }

  size_t len = strlen(editBuffer);
  if (len < sizeof(editBuffer) - 1) {
    editBuffer[len] = key[0];
    editBuffer[len + 1] = 0;
    uiDrawBoxKeypadValue();
  }
}

static void handleBoxTouch(int16_t x, int16_t y) {
  if (y >= BOX_ACTION_Y && y < BOX_ACTION_Y + BOX_ACTION_H) {
    if (x >= 6 && x < 106) {
      boxView = (boxView == BOX_VIEW_DIMENSIONS) ? BOX_VIEW_PARAMETERS
                                                 : BOX_VIEW_DIMENSIONS;
      state.pageDirty = true;
      return;
    }
    if (x >= 112 && x < 212) {
      boxGenerateFile();
      state.pageDirty = true;
      return;
    }
    if (x >= 218 && x < 338) {
      if (state.jobState == JOB_RUNNING) {
        jobPause();
      } else if (state.jobState == JOB_PAUSED) {
        jobResume();
      } else if (uiConfirm(CONFIRM_START_JOB, "Toca Ejecutar otra vez para confirmar")) {
        jobStart();
      }
      state.pageDirty = true;
      return;
    }
    if (x >= 344 && x < 474) {
      if (state.jobState == JOB_RUNNING || state.jobState == JOB_PAUSED) {
        if (uiConfirm(CONFIRM_STOP_JOB, "Toca Detener otra vez para confirmar")) {
          jobStop();
        }
      } else {
        grblSetMessage("No hay trabajo activo");
      }
      state.pageDirty = true;
      return;
    }
    return;
  }

  if (x >= BOX_PANEL_X && x < BOX_PANEL_X + BOX_PANEL_W) {
    if (boxView == BOX_VIEW_DIMENSIONS) {
      for (uint8_t i = 0; i < 3; i++) {
        int16_t rowY = 50 + i * 52;
        if (y >= rowY && y < rowY + 48) {
          uiOpenBoxField((BoxField)(BOX_FIELD_LENGTH + i));
          return;
        }
      }
    } else if (y >= 50 && y < 232) {
      uint8_t idx = (y - 50) / 26;
      if (idx < 7) {
        uiOpenBoxField((BoxField)(BOX_FIELD_TOOL + idx));
        return;
      }
    }
  }
}

static void handleJobAbortTouch(int16_t x, int16_t y) {
  if (y < 236 || y >= 268) return;

  if (x >= 12 && x < 152) {
    jobAbortM5();
  } else if (x >= 164 && x < 316) {
    if (uiConfirm(CONFIRM_ABORT_RESET,
                  "Toca RESET otra vez para reiniciar GRBL")) {
      jobAbortReset();
    }
  } else if (x >= 328 && x < 468) {
    jobAbortDismiss();
  }
}

void uiHandleTouch(int16_t x, int16_t y) {
  if (state.jobAbortPrompt) {
    handleJobAbortTouch(x, y);
    return;
  }

  if (editActive) {
    uiHandleKeypadTouch(x, y);
    return;
  }

  if (y >= NAV_Y && y < NAV_Y + NAV_H) {
    uint8_t page = (x < 6) ? 0 : (x - 6) / 118;
    if (page > 3) page = 3;
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
  } else if (state.page == PAGE_BOX) {
    handleBoxTouch(x, y);
  } else {
    handleSettingsTouch(x, y);
  }
}
