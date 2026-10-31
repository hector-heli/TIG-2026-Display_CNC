#include <WiFi.h>

static char linkLine[192];
static uint16_t linkLineLen = 0;
static WiFiClient tcpClient;
static unsigned long lastWifiAttemptMs = 0;
static unsigned long lastTcpAttemptMs = 0;
static unsigned long lastLinkLogMs = 0;
static unsigned long tcpConnectedMs = 0;
static unsigned long nextTcpAttemptMs = 0;
static unsigned long lastSeenStatusMs = 0;
static bool wifiAttemptActive = false;
static bool tcpUseFallbackPort = false;
static bool tcpHadStatus = false;
static uint8_t tcpFailCount = 0;

void linkBegin() {
  linkLineLen = 0;
  state.lastRxMs = millis();
  state.lastStatusMs = 0;
  state.linkOk = false;
  state.wifiConnected = false;
  state.tcpConnected = false;
  state.lastWifiReconnectMs = millis();
  lastWifiAttemptMs = millis() - WIFI_RECONNECT_MS;
  lastTcpAttemptMs = millis() - WIFI_RECONNECT_MS;
  lastLinkLogMs = 0;
  tcpConnectedMs = 0;
  nextTcpAttemptMs = 0;
  lastSeenStatusMs = 0;
  wifiAttemptActive = false;
  tcpUseFallbackPort = false;
  tcpHadStatus = false;
  tcpFailCount = 0;

  if (tcpClient.connected()) {
    tcpClient.stop();
  }

  if (state.linkMode == LINK_UART) {
    LinkSerial.begin(LINK_BAUD, SERIAL_8N1, CYD_RX_PIN, CYD_TX_PIN);
    LinkSerial.setTimeout(20);
  } else {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(false);
    WiFi.setSleep(false);
    delay(100);
    Serial.printf("WiFi target ssid='%s' -> %s:%u\n",
                  state.wifiSsid, state.dlcIp, (unsigned)state.dlcPort);
  }
}

void linkPoll() {
  if (state.lastStatusMs != lastSeenStatusMs) {
    lastSeenStatusMs = state.lastStatusMs;
    if (state.lastStatusMs != 0) {
      tcpHadStatus = true;
      tcpFailCount = 0;
      nextTcpAttemptMs = 0;
    }
  }

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
    unsigned long now = millis();

    if (WiFi.status() != WL_CONNECTED) {
      if (state.wifiConnected) {
        state.wifiConnected = false;
        state.statusDirty = true;
        Serial.println("WiFi disconnected");
      }
      if (tcpClient.connected()) {
        tcpClient.stop();
      }
      if (state.tcpConnected) {
        state.tcpConnected = false;
        state.statusDirty = true;
      }

      if (!wifiAttemptActive) {
        if (now - lastWifiAttemptMs >= WIFI_RECONNECT_MS) {
          lastWifiAttemptMs = now;
          if (state.wifiSsid[0] != 0) {
            WiFi.begin(state.wifiSsid, state.wifiPass);
            wifiAttemptActive = true;
            Serial.printf("WiFi begin ssid='%s' passLen=%u\n",
                          state.wifiSsid, (unsigned)strlen(state.wifiPass));
          } else {
            Serial.println("WiFi begin skipped: empty SSID");
          }
        }
      } else if (now - lastWifiAttemptMs >= WIFI_CONNECT_TIMEOUT_MS) {
        wifiAttemptActive = false;
        lastWifiAttemptMs = now;
        Serial.printf("WiFi timeout status=%d\n", (int)WiFi.status());
        WiFi.disconnect(false);
      }
    } else {
      wifiAttemptActive = false;
      if (!state.wifiConnected) {
        state.wifiConnected = true;
        state.statusDirty = true;
        Serial.printf("WiFi connected ip=%s rssi=%d\n",
                      WiFi.localIP().toString().c_str(), WiFi.RSSI());
      }

      if (!tcpClient.connected()) {
        if (state.tcpConnected) {
          state.tcpConnected = false;
          state.statusDirty = true;
          if (state.lastStatusMs) {
            Serial.printf("TCP disconnected statusAge=%lu\n", now - state.lastStatusMs);
          } else {
            Serial.printf("TCP disconnected no-status uptime=%lu\n", now);
          }
          if (!tcpHadStatus) {
            tcpFailCount++;
            if (tcpFailCount >= 3) {
              tcpFailCount = 0;
              nextTcpAttemptMs = now + TCP_RETRY_COOLDOWN_MS;
              Serial.printf("TCP cooldown %lu ms\n", (unsigned long)TCP_RETRY_COOLDOWN_MS);
            }
          }
        }
        if (now - lastTcpAttemptMs >= WIFI_RECONNECT_MS && now >= nextTcpAttemptMs) {
          lastTcpAttemptMs = now;
          if (state.dlcIp[0] != 0 && state.dlcPort > 0) {
            uint16_t portToTry = state.dlcPort;
            if (tcpUseFallbackPort) {
              portToTry = (state.dlcPort == DLC32_DEFAULT_PORT) ? DLC32_ALT_PORT
                                                                : DLC32_DEFAULT_PORT;
            }

            bool connected = tcpClient.connect(state.dlcIp, portToTry, 3000);
            Serial.printf("TCP connect %s:%u -> %s%s\n",
                          state.dlcIp, (unsigned)portToTry,
                          connected ? "OK" : "FAIL",
                          tcpUseFallbackPort ? " (fallback)" : "");

            if (connected) {
              tcpUseFallbackPort = false;
              if (portToTry != state.dlcPort) {
                state.dlcPort = portToTry;
                settingsSaveLink();
                Serial.printf("TCP port saved: %u\n", (unsigned)state.dlcPort);
              }
              state.tcpConnected = true;
              state.statusDirty = true;
              linkLineLen = 0;
              tcpConnectedMs = millis();
              tcpHadStatus = false;
            } else {
              tcpUseFallbackPort = !tcpUseFallbackPort;
              tcpFailCount++;
              if (tcpFailCount >= 3) {
                tcpFailCount = 0;
                nextTcpAttemptMs = now + TCP_RETRY_COOLDOWN_MS;
                Serial.printf("TCP cooldown %lu ms\n", (unsigned long)TCP_RETRY_COOLDOWN_MS);
              }
            }
          }
        }
      } else {
        if (!state.tcpConnected) {
          state.tcpConnected = true;
          state.statusDirty = true;
          linkLineLen = 0;
          tcpConnectedMs = now;
          tcpHadStatus = false;
        }
        if (now - tcpConnectedMs >= TCP_STALE_MS &&
            (state.lastStatusMs == 0 || now - state.lastStatusMs >= TCP_STALE_MS)) {
          if (state.lastStatusMs) {
            Serial.printf("TCP stale statusAge=%lu\n", now - state.lastStatusMs);
          } else {
            Serial.printf("TCP stale no-status uptime=%lu\n", now);
          }
          tcpClient.stop();
          state.tcpConnected = false;
          state.statusDirty = true;
          tcpConnectedMs = 0;
          lastTcpAttemptMs = now;
          if (!tcpHadStatus) {
            tcpFailCount++;
            if (tcpFailCount >= 3) {
              tcpFailCount = 0;
              nextTcpAttemptMs = now + TCP_RETRY_COOLDOWN_MS;
              Serial.printf("TCP cooldown %lu ms\n", (unsigned long)TCP_RETRY_COOLDOWN_MS);
            }
          }
        }
        if (state.tcpConnected) {
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

    if (millis() - lastLinkLogMs >= 1000) {
      lastLinkLogMs = millis();
      Serial.printf("Link status wifi=%d tcp=%u link=%u\n",
                    (int)WiFi.status(), (unsigned)state.tcpConnected,
                    (unsigned)state.linkOk);
    }
  }

  bool nowOk = (state.lastStatusMs != 0) &&
               (millis() - state.lastStatusMs < LINK_TIMEOUT_MS);

  if (nowOk != state.linkOk) {
    state.linkOk = nowOk;
    state.statusDirty = true;
    if (state.lastStatusMs) {
      Serial.printf("Link %s wifi=%d tcp=%u statusAge=%lu\n",
                    nowOk ? "up" : "down", (int)WiFi.status(),
                    (unsigned)state.tcpConnected,
                    millis() - state.lastStatusMs);
    } else {
      Serial.printf("Link %s wifi=%d tcp=%u no-status\n",
                    nowOk ? "up" : "down", (int)WiFi.status(),
                    (unsigned)state.tcpConnected);
    }
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
