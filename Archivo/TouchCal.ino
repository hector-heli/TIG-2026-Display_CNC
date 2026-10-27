void touchCalibrate(bool force) {
  uint16_t calData[5];
  bool calDataOK = false;

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS mount failed during calibration");
  }

  if (force && SPIFFS.exists(CALIBRATION_FILE)) {
    SPIFFS.remove(CALIBRATION_FILE);
  }

  if (!force && SPIFFS.exists(CALIBRATION_FILE)) {
    File f = SPIFFS.open(CALIBRATION_FILE, "r");
    if (f) {
      uint8_t storedRotation = 0xFF;
      if (f.size() == sizeof(calData) + 1 &&
          f.readBytes((char *)calData, sizeof(calData)) == sizeof(calData) &&
          f.readBytes((char *)&storedRotation, 1) == 1 &&
          storedRotation == DISPLAY_ROTATION) {
        calDataOK = true;
      }
      f.close();
    }
  }

  if (calDataOK) {
    tft.setTouch(calData);
    return;
  }

  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 10);
  tft.setTextFont(2);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Touch the corners as indicated");
  tft.println();
  tft.setTextFont(1);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.println("Keep the stylus still until the next point appears.");

  tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.println("Calibration complete");

  File f = SPIFFS.open(CALIBRATION_FILE, "w");
  if (f) {
    uint8_t rotation = DISPLAY_ROTATION;
    f.write((const unsigned char *)calData, sizeof(calData));
    f.write(&rotation, 1);
    f.close();
  }

  tft.setTouch(calData);
  delay(500);
}
