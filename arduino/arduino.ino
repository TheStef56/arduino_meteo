// #define SERIAL_DEBUG
#include "common.h"
#include "env.h"
#include "wifiMessage.h"
#include "sensors.h"

#define HOST "192.168.0.101"
#define PORT 9000
// 5 min
#define INTERVAL 5*1000

void setup() {
  IF_SERIAL_DEBUG(
    Serial.begin(9600);
    while (!Serial);
  )
  ledPrintInit();
  selectMode(5000, 300);
  setupBme();
  setupWifi();
}

void loop() {
  BMEData bme = getBMEdata();
  sendData(
    (Data){
    .windSpeedOpen  = getWindSpeedKmH(),//7.3f,
    .windSpeedClose = getWindSpeedKmH(),//9.3f,
    .windSpeedHigh  = getWindSpeedKmH(),//10.3f,
    .windSpeedLow   = getWindSpeedKmH(),//6.3f,
    .temperature    = bme.temperature,//30.2f,
    .humidity       = bme.humidity,//90.5f,
    .bmp            = bme.bmp,//1019.0f,
    .batteryVolts   = getBatteryVoltage(),
    .windDirection  = getWindDirectionDegrees(),
  });
  delay(INTERVAL);
}

void sendData(Data data) {
  int ret = sendWifiMessageEnc(HOST, PORT, (uint8_t*)(void*)&data, sizeof(Data), AES_KEY, sizeof(AES_KEY));
}