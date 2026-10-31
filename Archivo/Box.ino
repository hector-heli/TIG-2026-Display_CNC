#include <SPIFFS.h>

static File jobFile;
static unsigned long lastJobUiMs = 0;

static bool boxWriteLine(File &file, const char *line, uint16_t &count) {
  if (file.println(line) == 0) return false;
  if (count < 0xFFFF) count++;
  return true;
}

bool boxValidate() {
  if (state.boxLength < 5.0f || state.boxLength > 300.0f) {
    grblSetError("Largo de caja fuera de rango");
    return false;
  }
  if (state.boxWidth < 5.0f || state.boxWidth > 300.0f) {
    grblSetError("Ancho de caja fuera de rango");
    return false;
  }
  if (state.boxHeight < 1.0f || state.boxHeight > 100.0f) {
    grblSetError("Altura de caja fuera de rango");
    return false;
  }
  if (state.boxToolDia < 0.5f || state.boxToolDia > 20.0f) {
    grblSetError("Diametro de fresa fuera de rango");
    return false;
  }

  float minDim = state.boxLength < state.boxWidth ? state.boxLength : state.boxWidth;
  if (state.boxToolDia >= minDim) {
    grblSetError("La fresa es demasiado grande para la caja");
    return false;
  }
  if (state.boxStepoverPct < 5.0f || state.boxStepoverPct > 80.0f) {
    grblSetError("Paso lateral fuera de rango");
    return false;
  }
  if (state.boxFeed < 50.0f || state.boxFeed > 3000.0f) {
    grblSetError("Avance fuera de rango");
    return false;
  }
  if (state.boxRpm < 100 || state.boxRpm > state.maxRpm) {
    grblSetError("RPM fuera de rango");
    return false;
  }
  if (state.boxSafeZ < 1.0f || state.boxSafeZ > 50.0f) {
    grblSetError("Z seguro fuera de rango");
    return false;
  }
  if (state.boxDepthStep < 0.1f || state.boxDepthStep > 10.0f ||
      state.boxDepthStep > state.boxHeight) {
    grblSetError("Bajada por pasada fuera de rango");
    return false;
  }
  if (state.boxPlungeFeed < 10.0f || state.boxPlungeFeed > 1000.0f) {
    grblSetError("Avance de penetracion fuera de rango");
    return false;
  }
  return true;
}

bool boxGenerateFile() {
  if (state.jobState == JOB_RUNNING || state.jobState == JOB_PAUSED) {
    grblSetMessage("Deten el trabajo antes de generar");
    return false;
  }
  if (!boxValidate()) return false;

  float xMax = state.boxLength * 0.5f - state.boxToolDia * 0.5f;
  float yMax = state.boxWidth * 0.5f - state.boxToolDia * 0.5f;
  if (xMax <= 0.0f || yMax <= 0.0f) {
    grblSetError("La fresa no cabe en la caja");
    return false;
  }

  float stepY = state.boxToolDia * state.boxStepoverPct / 100.0f;
  if (stepY < 0.1f) stepY = 0.1f;

  float ySpan = 2.0f * yMax;
  int yPasses = 1;
  if (ySpan > 0.0001f) {
    yPasses = (int)ceilf(ySpan / stepY) + 1;
    if (yPasses < 2) yPasses = 2;
  }
  float passStep = (yPasses > 1) ? (ySpan / (float)(yPasses - 1)) : 0.0f;

  int zLayers = (int)ceilf(state.boxHeight / state.boxDepthStep);
  if (zLayers < 1) zLayers = 1;
  float zStep = state.boxHeight / (float)zLayers;

  long estimatedLines = (long)zLayers * (long)yPasses * 2L + 12L;
  if (estimatedLines > BOX_MAX_LINES) {
    grblSetError("Rutina demasiado grande; aumenta fresa o paso");
    return false;
  }

  SPIFFS.remove(BOX_FILE_PATH);
  File file = SPIFFS.open(BOX_FILE_PATH, FILE_WRITE);
  if (!file) {
    grblSetError("No se pudo crear /box.nc");
    return false;
  }

  char buf[96];
  uint16_t lineCount = 0;
  bool ok = true;
  int direction = 1;

  ok = boxWriteLine(file, "G21", lineCount) && ok;
  ok = boxWriteLine(file, "G90", lineCount) && ok;
  ok = boxWriteLine(file, "G54", lineCount) && ok;

  snprintf(buf, sizeof(buf), "G0 Z%.3f", state.boxSafeZ);
  ok = boxWriteLine(file, buf, lineCount) && ok;

  snprintf(buf, sizeof(buf), "G0 X%.3f Y%.3f", -xMax, -yMax);
  ok = boxWriteLine(file, buf, lineCount) && ok;

  snprintf(buf, sizeof(buf), "S%d M3", state.boxRpm);
  ok = boxWriteLine(file, buf, lineCount) && ok;
  ok = boxWriteLine(file, "G4 P1", lineCount) && ok;

  float approachZ = state.boxSafeZ > 2.0f ? 1.0f : state.boxSafeZ * 0.5f;
  snprintf(buf, sizeof(buf), "G0 Z%.3f", approachZ);
  ok = boxWriteLine(file, buf, lineCount) && ok;

  for (int layer = 1; layer <= zLayers && ok; layer++) {
    float z = -(zStep * (float)layer);
    snprintf(buf, sizeof(buf), "G1 Z%.3f F%.0f", z, state.boxPlungeFeed);
    ok = boxWriteLine(file, buf, lineCount) && ok;

    for (int pass = 0; pass < yPasses && ok; pass++) {
      float targetX = direction > 0 ? xMax : -xMax;
      snprintf(buf, sizeof(buf), "G1 X%.3f F%.0f", targetX, state.boxFeed);
      ok = boxWriteLine(file, buf, lineCount) && ok;

      if (pass < yPasses - 1) {
        float nextY = -yMax + (float)(pass + 1) * passStep;
        snprintf(buf, sizeof(buf), "G1 Y%.3f F%.0f", nextY, state.boxFeed);
        ok = boxWriteLine(file, buf, lineCount) && ok;
      }
      direction = -direction;
    }
  }

  snprintf(buf, sizeof(buf), "G0 Z%.3f", state.boxSafeZ);
  ok = boxWriteLine(file, buf, lineCount) && ok;
  ok = boxWriteLine(file, "M5", lineCount) && ok;
  ok = boxWriteLine(file, "G0 X0 Y0", lineCount) && ok;
  ok = boxWriteLine(file, "M30", lineCount) && ok;

  file.close();

  if (!ok) {
    SPIFFS.remove(BOX_FILE_PATH);
    state.jobTotalLines = 0;
    state.jobLine = 0;
    grblSetError("Error escribiendo /box.nc");
    return false;
  }

  state.jobTotalLines = lineCount;
  state.jobLine = 0;
  state.jobState = JOB_IDLE;
  state.jobDirty = true;
  grblSetMessage("G-code generado");
  return true;
}

bool jobStart() {
  if (state.jobState == JOB_RUNNING || state.jobState == JOB_PAUSED) {
    grblSetMessage("Ya hay un trabajo activo");
    return false;
  }
  if (!state.linkOk) {
    grblSetError("Sin enlace con DLC32");
    return false;
  }
  if (state.alarm) {
    grblSetError("Alarma activa; usa $X");
    return false;
  }
  if (cmd.busy) {
    grblSetMessage("Comando en curso");
    return false;
  }

  if (!SPIFFS.exists(BOX_FILE_PATH) || state.jobTotalLines == 0) {
    if (!boxGenerateFile()) return false;
  }

  jobFile = SPIFFS.open(BOX_FILE_PATH, "r");
  if (!jobFile) {
    grblSetError("No se pudo abrir /box.nc");
    return false;
  }

  state.jobLine = 0;
  state.jobState = JOB_RUNNING;
  state.jobAbortPrompt = false;
  state.jobDirty = true;
  lastJobUiMs = millis();
  grblSetMessage("Trabajo iniciado");
  return true;
}

static void jobFinish() {
  if (jobFile) jobFile.close();
  state.jobState = JOB_IDLE;
  state.jobAbortPrompt = false;
  state.jobDirty = true;
  grblSetMessage("Trabajo finalizado");
}

static void jobAbort(const char *reason) {
  if (state.jobState != JOB_RUNNING && state.jobState != JOB_PAUSED) return;
  linkSendRealtime('!');
  if (jobFile) jobFile.close();
  cmd.busy = false;
  cmd.spindlePending = false;
  state.jobState = JOB_ERROR;
  state.jobAbortPrompt = true;
  state.jobDirty = true;
  state.pageDirty = true;
  grblSetError(reason ? reason : "Trabajo detenido");
}

void jobAbortM5() {
  state.confirm = CONFIRM_NONE;
  if (grblSendCommand("M5")) {
    cmd.spindlePending = true;
    cmd.pendingSpindleDir = SPINDLE_STOP;
    grblSetMessage("M5 enviado; verifica el husillo");
  }
  state.jobDirty = true;
}

void jobAbortReset() {
  state.confirm = CONFIRM_NONE;
  if (!state.linkOk) {
    grblSetError("Sin enlace; usa el paro fisico");
    state.jobDirty = true;
    return;
  }
  linkSendRealtime(0x18);
  cmd.busy = false;
  cmd.spindlePending = false;
  grblSetMessage("Reset GRBL enviado; rehome antes de continuar");
  state.jobDirty = true;
}

void jobAbortDismiss() {
  state.confirm = CONFIRM_NONE;
  state.jobAbortPrompt = false;
  state.jobState = JOB_IDLE;
  state.jobLine = 0;
  state.jobDirty = true;
  state.pageDirty = true;
  grblSetMessage("Revisa la maquina antes de continuar");
}

void jobOnGrblError(const char *reason) {
  if (state.jobState == JOB_RUNNING || state.jobState == JOB_PAUSED) {
    jobAbort(reason);
  }
}

void jobPause() {
  if (state.jobState != JOB_RUNNING) return;
  linkSendRealtime('!');
  state.jobState = JOB_PAUSED;
  state.jobDirty = true;
  grblSetMessage("Trabajo en pausa");
}

void jobResume() {
  if (state.jobState != JOB_PAUSED) return;
  linkSendRealtime('~');
  state.jobState = JOB_RUNNING;
  state.jobDirty = true;
  grblSetMessage("Trabajo reanudado");
}

void jobStop() {
  if (state.jobState != JOB_RUNNING && state.jobState != JOB_PAUSED) {
    grblSetMessage("No hay trabajo activo");
    return;
  }
  linkSendRealtime('!');
  linkSendRealtime(0x18);
  if (jobFile) jobFile.close();
  cmd.busy = false;
  cmd.spindlePending = false;
  state.jobState = JOB_IDLE;
  state.jobAbortPrompt = false;
  state.jobLine = 0;
  state.jobDirty = true;
  state.pageDirty = true;
  grblSetMessage("Trabajo detenido");
}

void jobProcess() {
  if (state.jobState != JOB_RUNNING) return;

  if (!state.linkOk || (state.linkMode == LINK_WIFI && !state.tcpConnected)) {
    unsigned long statusAge = state.lastStatusMs ? millis() - state.lastStatusMs : millis();
    Serial.printf("Job abort link: link=%u tcp=%u statusAge=%lu line=%u state=%s\n",
                  (unsigned)state.linkOk, (unsigned)state.tcpConnected,
                  statusAge, (unsigned)state.jobLine, state.grblState);
    const char *reason = (state.linkMode == LINK_WIFI && !state.tcpConnected)
                         ? "TCP desconectado; trabajo detenido"
                         : "Sin status GRBL; trabajo detenido";
    jobAbort(reason);
    return;
  }
  if (state.alarm) {
    jobAbort("Alarma durante el trabajo");
    return;
  }
  if (cmd.busy) return;

  if (!jobFile) {
    jobFinish();
    return;
  }

  char line[96];
  bool sent = false;

  while (jobFile.available()) {
    size_t len = jobFile.readBytesUntil('\n', line, sizeof(line) - 1);
    line[len] = 0;

    while (len > 0 && (line[len - 1] == '\r' || line[len - 1] == ' ' ||
                       line[len - 1] == '\t')) {
      line[--len] = 0;
    }
    if (len == 0 || line[0] == ';' || line[0] == '(') continue;

    if (!grblSendCommand(line)) return;
    state.jobLine++;
    sent = true;
    break;
  }

  if (!sent && !jobFile.available() && !cmd.busy) {
    jobFinish();
    return;
  }

  if (sent && millis() - lastJobUiMs >= 250) {
    lastJobUiMs = millis();
    state.jobDirty = true;
  }
}
