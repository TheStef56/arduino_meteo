#define SERIAL_DEBUG
#include "ctx.h"
#include "env.h"
#include "wifiMessage.h"
#include "simMessage.h"
#include "sensors.h"
// EXAMPLE env.h:

// char REMOTE_HOST[] = "example.com";
// int REMOTE_PORT = 9000;
// char WIFI_HOST[] = "192.168.0.1";
// int WIFI_PORT = 9000;
// #define WIFI_SSID "example"
// #define WIFI_PASSWORD "12345678"
// #define APN "example.it"
// #define SIM_PIN "1234"

// uint8_t AES_KEY[32] = {
//   0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
//   0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
//   0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
//   0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01
// };

#define MOSFET_PIN 4

size_t count = 0;
float windMean = 0.f;
float windOpen = 0.f;
float windMax = 0.f;
float windMin = 9999.f;

void (*sendData)(WindData data);

void setup() {
  ledPrintInit();
  
  // switch off mosfet
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(4, LOW);

  setupBme();
  selectMode(5000, 300);

  IF_SERIAL_DEBUG(
    Serial.begin(9600);
    while (!Serial) {
      ledPrint("SERIAL FAIL", true);
    };
  );

  if (SETTINGS & SIM) {
    Serial1.begin(115200);
    sendData = sendSimData;
  }
  if (SETTINGS & WIFI) {
    setupWifi();
    sendData = sendWifiData;
  }
  if (SETTINGS & SIM_NO_SHUTDOWN) digitalWrite(MOSFET_PIN, HIGH);
}

void loop() {
  if (count*WIND_MEASURING_INTERVAL > DATA_SENDING_INTERVAL) {
    BMEData bme = getBMEdata();
    sendData(
      (WindData){
      .windSpeedOpen  = windOpen,
      .windSpeedClose = getWindSpeedKmH(),
      .windSpeedHigh  = windMax,
      .windSpeedLow   = windMin,
      .windMean       = (float)windMean/count,
      .temperature    = bme.temperature,
      .humidity       = bme.humidity,
      .bmp            = bme.bmp,
      .batteryVolts   = getBatteryVoltage(),
      .windDirection  = getWindDirectionDegrees(),
    });
    count    = 0;
    windMean = 0.f;
    windOpen = 0.f;
    windMax  = 0.f;
    windMin  = 9999.f;
  }
  float wSpeed = getWindSpeedKmH();
  windMean += wSpeed;
  if (wSpeed > windMax) windMax  = wSpeed;
  if (wSpeed < windMin) windMin  = wSpeed;
  if (count == 0) windOpen       = wSpeed;
  delay(WIND_MEASURING_INTERVAL);
  count++;
}

void sendSimData(WindData data) {
  bool success = false;
  bool new_attempt = false;
  do {
    if (new_attempt) {
      // if the module failed to send the data and reached timeout, we are rebooting it, even if we are in SIM_NO_SHUTDOWN mode
      digitalWrite(MOSFET_PIN, LOW);
      delay(5000);  // sleep for 5 seconds
      digitalWrite(MOSFET_PIN, HIGH);
    }
    if (!(SETTINGS & SIM_NO_SHUTDOWN)) digitalWrite(MOSFET_PIN, HIGH);
    success = sendSimMessageEnc(REMOTE_HOST, REMOTE_PORT, (uint8_t*)(void*)&data, sizeof(WindData), AES_KEY, sizeof(AES_KEY));
    if (!(SETTINGS & SIM_NO_SHUTDOWN)) digitalWrite(MOSFET_PIN, LOW);
    new_attempt = true;
  } while (!success); 
}

void sendWifiData(WindData data) {
  sendWifiMessageEnc(WIFI_HOST, WIFI_PORT, (uint8_t*)(void*)&data, sizeof(WindData), AES_KEY, sizeof(AES_KEY));
}