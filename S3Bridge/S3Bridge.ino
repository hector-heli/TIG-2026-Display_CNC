#define USBHOSTSERIAL_BUFFERSIZE 4096

#include <Arduino.h>
#include <USBHostSerial.h>

#define CYD_TX_PIN 4
#define CYD_RX_PIN 5
#define CYD_BAUD 115200
#define USB_BAUD 115200
#define STATUS_LED 48

HardwareSerial CydSerial(1);
USBHostSerial UsbSerial;

static uint8_t usbBuf[256];
static uint8_t cydBuf[256];
static unsigned long lastLedMs = 0;

static void usbLog(const char *msg) {
  Serial.printf("[USB] %s\n", msg);
}

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

  CydSerial.begin(CYD_BAUD, SERIAL_8N1, CYD_RX_PIN, CYD_TX_PIN);

  UsbSerial.setLogger(usbLog);
  UsbSerial.begin(USB_BAUD, 0, 0, 8);

  Serial.println("S3 Grbl bridge ready");
  Serial.printf("CYD UART TX=%d RX=%d @ %d\n", CYD_TX_PIN, CYD_RX_PIN, CYD_BAUD);
}

void loop() {
  if (UsbSerial.available()) {
    size_t n = UsbSerial.read(usbBuf, sizeof(usbBuf));
    if (n > 0) {
      CydSerial.write(usbBuf, n);
    }
  }

  if (CydSerial.available()) {
    size_t n = CydSerial.read(cydBuf, sizeof(cydBuf));
    size_t sent = 0;

    while (sent < n) {
      if (!UsbSerial) break;
      size_t w = UsbSerial.write(cydBuf + sent, n - sent);
      if (w == 0) {
        delay(1);
      } else {
        sent += w;
      }
    }
  }

  unsigned long now = millis();
  if (now - lastLedMs >= 500) {
    lastLedMs = now;
    digitalWrite(STATUS_LED, UsbSerial ? HIGH : LOW);
  }

  delay(1);
}
