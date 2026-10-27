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
  strncpy(state.wifiSsid, ssid.c_str(), sizeof(state.wifiSsid) - 1);
  strncpy(state.wifiPass, pass.c_str(), sizeof(state.wifiPass) - 1);
  state.wifiSsid[sizeof(state.wifiSsid) - 1] = 0;
  state.wifiPass[sizeof(state.wifiPass) - 1] = 0;
  String ip = prefs.getString("dlcip", DLC32_DEFAULT_IP);
  strncpy(state.dlcIp, ip.c_str(), sizeof(state.dlcIp) - 1);
  state.dlcIp[sizeof(state.dlcIp) - 1] = 0;
  state.dlcPort = prefs.getUShort("dlcport", DLC32_DEFAULT_PORT);
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
