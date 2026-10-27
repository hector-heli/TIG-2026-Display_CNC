static char linkLine[192];
static uint16_t linkLineLen = 0;

void linkBegin() {
  LinkSerial.begin(LINK_BAUD, SERIAL_8N1, CYD_RX_PIN, CYD_TX_PIN);
  LinkSerial.setTimeout(20);
  state.lastRxMs = millis();
  state.lastStatusMs = 0;
  state.linkOk = false;
}

void linkPoll() {
  while (LinkSerial.available()) {
    int c = LinkSerial.read();
    if (c < 0) break;

    state.lastRxMs = millis();

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      if (linkLineLen > 0) {
        linkLine[linkLineLen] = 0;
        grblParseLine(linkLine);
        linkLineLen = 0;
      }
      continue;
    }

    if (linkLineLen < sizeof(linkLine) - 1) {
      linkLine[linkLineLen++] = (char)c;
    } else {
      linkLineLen = 0;
    }
  }

  bool nowOk = (state.lastStatusMs != 0) &&
               (millis() - state.lastStatusMs < LINK_TIMEOUT_MS);

  if (nowOk != state.linkOk) {
    state.linkOk = nowOk;
    state.statusDirty = true;
  }
}

void linkSendLine(const char *text) {
  if (text == nullptr || *text == 0) return;

  LinkSerial.write((const uint8_t *)text, strlen(text));
  LinkSerial.write('\n');
}

void linkSendRealtime(uint8_t c) {
  LinkSerial.write(c);
}
