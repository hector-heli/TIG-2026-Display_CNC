#include <WiFi.h>

static char linkLine[192];
static uint16_t linkLineLen = 0;
static WiFiClient tcpClient;
static unsigned long lastWifiAttemptMs = 0;

void linkBegin() {
  linkLineLen = 0;
  state.lastRxMs = millis();
  state.lastStatusMs = 0;
  state.linkOk = false;
  state.wifiConnected = false;
  state.tcpConnected = false;
  state.lastWifiReconnectMs = millis();
  lastWifiAttemptMs = 0;

  if (tcpClient.connected()) {
    tcpClient.stop();
  }

  if (state.linkMode == LINK_UART) {
    LinkSerial.begin(LINK_BAUD, SERIAL_8N1, CYD_RX_PIN, CYD_TX_PIN);
    LinkSerial.setTimeout(20);
  } else {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(true);
    delay(100);
    lastWifiAttemptMs = millis() - WIFI_RECONNECT_MS;
  }
}

void linkPoll() {
  if (state.linkMode == LINK_UART) {
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
  } else {
    // WiFi mode
    if (WiFi.status() != WL_CONNECTED) {
      if (state.wifiConnected) {
        state.wifiConnected = false;
        state.statusDirty = true;
      }
      if (tcpClient.connected()) {
        tcpClient.stop();
      }
      if (state.tcpConnected) {
        state.tcpConnected = false;
        state.statusDirty = true;
      }
      if (millis() - lastWifiAttemptMs >= WIFI_RECONNECT_MS) {
        lastWifiAttemptMs = millis();
        if (state.wifiSsid[0] != 0) {
          WiFi.begin(state.wifiSsid, state.wifiPass);
        }
      }
    } else {
      if (!state.wifiConnected) {
        state.wifiConnected = true;
        state.statusDirty = true;
      }
      if (!tcpClient.connected()) {
        if (state.tcpConnected) {
          state.tcpConnected = false;
          state.statusDirty = true;
        }
        if (millis() - lastWifiAttemptMs >= WIFI_RECONNECT_MS) {
          lastWifiAttemptMs = millis();
          if (state.dlcIp[0] != 0 && state.dlcPort > 0) {
            tcpClient.connect(state.dlcIp, state.dlcPort);
          }
        }
      } else {
        if (!state.tcpConnected) {
          state.tcpConnected = true;
          state.statusDirty = true;
          linkLineLen = 0;
        }
        while (tcpClient.available()) {
          int c = tcpClient.read();
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
      }
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

  if (state.linkMode == LINK_UART) {
    LinkSerial.write((const uint8_t *)text, strlen(text));
    LinkSerial.write('\n');
  } else {
    if (tcpClient.connected()) {
      tcpClient.print(text);
      tcpClient.print('\n');
    }
  }
}

void linkSendRealtime(uint8_t c) {
  if (state.linkMode == LINK_UART) {
    LinkSerial.write(c);
  } else {
    if (tcpClient.connected()) {
      tcpClient.write(c);
    }
  }
}
