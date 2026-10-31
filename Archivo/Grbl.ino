static unsigned long grblLastPollMs = 0;

static bool parseFloats(const char *s, float *out, uint8_t count) {
  const char *p = s;
  for (uint8_t i = 0; i < count; i++) {
    char *end = nullptr;
    float v = strtod(p, &end);
    if (end == p) return false;
    out[i] = v;
    p = end;
    if (i < count - 1) {
      if (*p != ',') return false;
      p++;
    }
  }
  return true;
}

static bool parseInts(const char *s, int *out, uint8_t count) {
  const char *p = s;
  for (uint8_t i = 0; i < count; i++) {
    char *end = nullptr;
    long v = strtol(p, &end, 10);
    if (end == p) return false;
    out[i] = (int)v;
    p = end;
    if (i < count - 1) {
      if (*p != ',') return false;
      p++;
    }
  }
  return true;
}

void grblSetMessage(const char *msg) {
  if (msg == nullptr) msg = "";
  strncpy(state.message, msg, sizeof(state.message) - 1);
  state.message[sizeof(state.message) - 1] = 0;
  state.messageMs = millis();
  state.statusDirty = true;
}

void grblSetError(const char *msg) {
  if (msg == nullptr) msg = "";
  strncpy(state.lastError, msg, sizeof(state.lastError) - 1);
  state.lastError[sizeof(state.lastError) - 1] = 0;
  state.lastErrorMs = millis();
  grblSetMessage(msg);
  jobOnGrblError(msg);
}

void grblBegin() {
  grblLastPollMs = millis() - STATUS_POLL_MS;
}

void grblProcess() {
  unsigned long now = millis();

  if (now - grblLastPollMs >= STATUS_POLL_MS) {
    grblLastPollMs = now;
    linkSendRealtime('?');
  }

  bool jobActive = (state.jobState == JOB_RUNNING ||
                    state.jobState == JOB_PAUSED);
  bool moving = jobActive && state.linkOk &&
                (strncmp(state.grblState, "Run", 3) == 0 ||
                 strncmp(state.grblState, "Jog", 3) == 0 ||
                 strncmp(state.grblState, "Home", 4) == 0);
  unsigned long timeoutMs = jobActive
                            ? (moving ? 0UL : JOB_ACK_TIMEOUT_MS)
                            : CMD_TIMEOUT_MS;

  if (cmd.busy && timeoutMs > 0 && now - cmd.sentMs > timeoutMs) {
    Serial.printf("GRBL timeout cmd='%s' state=%s elapsed=%lu\n",
                  cmd.text, state.grblState, now - cmd.sentMs);
    cmd.busy = false;
    cmd.spindlePending = false;
    grblSetError("Timeout de comando");
  }
}

bool grblSendCommand(const char *text, bool motion) {
  if (text == nullptr || *text == 0) return false;

  if (cmd.busy) {
    grblSetMessage("Ocupado, espera ok/error");
    return false;
  }

  if (!state.linkOk) {
    grblSetError("Sin enlace con DLC32");
    return false;
  }

  strncpy(cmd.text, text, sizeof(cmd.text) - 1);
  cmd.text[sizeof(cmd.text) - 1] = 0;
  cmd.busy = true;
  cmd.motion = motion;
  cmd.sentMs = millis();

  linkSendLine(cmd.text);
  state.statusDirty = true;
  return true;
}

void grblParseStatus(const char *line) {
  char buf[192];
  strncpy(buf, line, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = 0;

  char *p = buf;
  if (*p == '<') p++;
  char *gt = strchr(p, '>');
  if (gt) *gt = 0;

  char *save = nullptr;
  char *tok = strtok_r(p, "|", &save);
  if (tok == nullptr) return;

  strncpy(state.grblState, tok, sizeof(state.grblState) - 1);
  state.grblState[sizeof(state.grblState) - 1] = 0;
  state.alarm = (strcmp(tok, "Alarm") == 0);
  state.homing = (strcmp(tok, "Home") == 0);
  state.wposValid = false;

  while ((tok = strtok_r(nullptr, "|", &save)) != nullptr) {
    if (strncmp(tok, "MPos:", 5) == 0) {
      parseFloats(tok + 5, state.mpos, 3);
    } else if (strncmp(tok, "WPos:", 5) == 0) {
      state.wposValid = parseFloats(tok + 5, state.wpos, 3);
    } else if (strncmp(tok, "FS:", 3) == 0) {
      float fs[2];
      if (parseFloats(tok + 3, fs, 2)) {
        state.feed = fs[0];
        state.spindle = fs[1];
      }
    } else if (strncmp(tok, "F:", 2) == 0) {
      float f;
      if (parseFloats(tok + 2, &f, 1)) state.feed = f;
    } else if (strncmp(tok, "S:", 2) == 0) {
      float s;
      if (parseFloats(tok + 2, &s, 1)) state.spindle = s;
    } else if (strncmp(tok, "Ov:", 3) == 0) {
      int ov[3];
      if (parseInts(tok + 3, ov, 3)) {
        state.feedOv = ov[0];
        state.rapidOv = ov[1];
        state.spindleOv = ov[2];
      }
    }
  }

  state.lastStatusMs = millis();
  state.statusDirty = true;
}

void grblParseLine(const char *line) {
  if (line == nullptr || *line == 0) return;

  if (line[0] == '<') {
    grblParseStatus(line);
    return;
  }

  if (strcmp(line, "ok") == 0) {
    cmd.busy = false;
    if (cmd.spindlePending) {
      state.spindleDir = cmd.pendingSpindleDir;
      cmd.spindlePending = false;
      state.pageDirty = true;
      if (state.spindleDir == SPINDLE_CCW) {
        grblSetMessage("M4 CCW activo");
      } else if (state.spindleDir == SPINDLE_CW) {
        grblSetMessage("M3 CW activo");
      } else {
        grblSetMessage("Husillo detenido");
      }
    }
    state.statusDirty = true;
    return;
  }

  if (strncmp(line, "error:", 6) == 0) {
    cmd.busy = false;
    cmd.spindlePending = false;
    grblSetError(line);
    return;
  }

  if (strncmp(line, "ALARM:", 6) == 0) {
    cmd.busy = false;
    cmd.spindlePending = false;
    state.alarm = true;
    strncpy(state.grblState, "Alarm", sizeof(state.grblState) - 1);
    state.grblState[sizeof(state.grblState) - 1] = 0;
    grblSetError(line);
    return;
  }

  if (strncmp(line, "[MSG:", 5) == 0) {
    const char *msg = line + 5;
    size_t len = strlen(msg);
    if (len > 0 && msg[len - 1] == ']') len--;
    char tmp[48];
    if (len >= sizeof(tmp)) len = sizeof(tmp) - 1;
    memcpy(tmp, msg, len);
    tmp[len] = 0;
    grblSetMessage(tmp);
    return;
  }

  if (line[0] == '[') {
    return;
  }

  if (strncmp(line, "Grbl", 4) == 0) {
    grblSetMessage(line);
  }
}
