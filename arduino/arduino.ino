#include "common.h"
#include "env.h"
#include "wifiMessage.h"
#include "sensors.h"

size_t count = 0;
float windMean = 0.f;
float windOpen = 0.f;
float windMax = 0.f;
float windMin = 9999.f;

void setup() {
  ledPrintInit();
  IF_SERIAL_DEBUG(
    Serial.begin(9600);
    while (!Serial) {
      ledPrint("SERIAL FAIL", true);
    };
  )
  setupBme();
  selectMode(5000, 300);
  setupWifi();
}

void loop() {
  if (count*WIND_MEASURING_INTERVAL > DATA_SENDING_INTERVAL) {
    BMEData bme = getBMEdata();
    sendData(
      (Data){
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

void sendData(Data data) {
  int ret = sendWifiMessageEnc(HOST, PORT, (uint8_t*)(void*)&data, sizeof(Data), AES_KEY, sizeof(AES_KEY));
}