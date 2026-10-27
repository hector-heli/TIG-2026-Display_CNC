void settingsLoad() {
  prefs.begin("pendant", false);

  state.jogStep = prefs.getFloat("step", DEFAULT_JOG_STEP);
  state.jogFeed = prefs.getFloat("feed", DEFAULT_JOG_FEED);
  state.maxRpm = prefs.getInt("maxrpm", DEFAULT_MAX_RPM);
  state.targetRpm = prefs.getInt("rpm", DEFAULT_TARGET_RPM);

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
}

void settingsSave() {
  prefs.putFloat("step", state.jogStep);
  prefs.putFloat("feed", state.jogFeed);
  prefs.putInt("maxrpm", state.maxRpm);
  prefs.putInt("rpm", state.targetRpm);
}
