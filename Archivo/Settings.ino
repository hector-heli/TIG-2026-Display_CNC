void settingsLoad() {
  prefs.begin("pendant", false);

  state.jogStep = prefs.getFloat("step", DEFAULT_JOG_STEP);
  state.jogFeed = prefs.getFloat("feed", DEFAULT_JOG_FEED);
  state.maxRpm = prefs.getInt("maxrpm", DEFAULT_MAX_RPM);
  state.targetRpm = prefs.getInt("rpm", DEFAULT_TARGET_RPM);

  // Link configuration
  state.linkMode = (LinkMode)prefs.getUChar("linkmode", LINK_MODE_DEFAULT);
  String ssid = prefs.getString("wifissid", WIFI_DEFAULT_SSID);
  String pass = prefs.getString("wifipass", WIFI_DEFAULT_PASS);
  String ip = prefs.getString("dlcip", DLC32_DEFAULT_IP);
  uint16_t port = prefs.getUShort("dlcport", DLC32_DEFAULT_PORT);

  // Firmware anterior podía guardar credenciales vacías en NVS. Si la SSID
  // está vacía, toda la configuración de enlace es inválida: restaurar los
  // valores de fábrica en lugar de dejar el WiFi deshabilitado para siempre.
  bool linkMigrated = false;
  if (ssid.length() == 0) {
    ssid = WIFI_DEFAULT_SSID;
    pass = WIFI_DEFAULT_PASS;
    ip = DLC32_DEFAULT_IP;
    port = DLC32_DEFAULT_PORT;
    linkMigrated = true;
  } else if (ssid == WIFI_DEFAULT_SSID && pass.length() == 0) {
    pass = WIFI_DEFAULT_PASS;
    linkMigrated = true;
  }

  if (ip.length() == 0) {
    ip = DLC32_DEFAULT_IP;
    linkMigrated = true;
  }

  // 80 es HTTP, no el flujo GRBL del DLC32.
  if (port == 0 || port == DLC32_HTTP_PORT) {
    port = DLC32_DEFAULT_PORT;
    linkMigrated = true;
  }

  if (linkMigrated) {
    prefs.putString("wifissid", ssid);
    prefs.putString("wifipass", pass);
    prefs.putString("dlcip", ip);
    prefs.putUShort("dlcport", port);
  }

  strncpy(state.wifiSsid, ssid.c_str(), sizeof(state.wifiSsid) - 1);
  strncpy(state.wifiPass, pass.c_str(), sizeof(state.wifiPass) - 1);
  state.wifiSsid[sizeof(state.wifiSsid) - 1] = 0;
  state.wifiPass[sizeof(state.wifiPass) - 1] = 0;
  strncpy(state.dlcIp, ip.c_str(), sizeof(state.dlcIp) - 1);
  state.dlcIp[sizeof(state.dlcIp) - 1] = 0;
  state.dlcPort = port;
  state.wifiConnected = false;
  state.tcpConnected = false;
  state.lastWifiReconnectMs = 0;

  if (state.jogStep <= 0.0f) state.jogStep = DEFAULT_JOG_STEP;
  if (state.jogFeed < 50.0f) state.jogFeed = 50.0f;
  if (state.maxRpm < 100) state.maxRpm = DEFAULT_MAX_RPM;
  if (state.targetRpm < 0) state.targetRpm = 0;
  if (state.targetRpm > state.maxRpm) state.targetRpm = state.maxRpm;

  state.page = PAGE_MAIN;
  state.spindleDir = SPINDLE_STOP;
  state.feedOv = 100;
  state.spindleOv = 100;
  state.rapidOv = 100;
  state.message[0] = 0;
  state.lastError[0] = 0;
  state.confirm = CONFIRM_NONE;
  state.wposValid = false;

  strncpy(state.grblState, "?", sizeof(state.grblState) - 1);
  state.grblState[sizeof(state.grblState) - 1] = 0;

  prefs.end();

  Serial.printf("Link config: mode=%s ssid='%s' passLen=%u target=%s:%u%s\n",
                state.linkMode == LINK_WIFI ? "WiFi" : "UART",
                state.wifiSsid, (unsigned)strlen(state.wifiPass),
                state.dlcIp, (unsigned)state.dlcPort,
                linkMigrated ? " (migrated)" : "");
}

void settingsSave() {
  prefs.begin("pendant", false);
  prefs.putFloat("step", state.jogStep);
  prefs.putFloat("feed", state.jogFeed);
  prefs.putInt("maxrpm", state.maxRpm);
  prefs.putInt("rpm", state.targetRpm);
  prefs.end();
}

void settingsSaveLink() {
  prefs.begin("pendant", false);
  prefs.putUChar("linkmode", state.linkMode);
  prefs.putString("wifissid", state.wifiSsid);
  prefs.putString("wifipass", state.wifiPass);
  prefs.putString("dlcip", state.dlcIp);
  prefs.putUShort("dlcport", state.dlcPort);
  prefs.end();
}
