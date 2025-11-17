#define SERIAL_DEBUG
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
  setupBme();
  setupWifi();
}

void loop() {
  BMEData bme = getBMEdata();
  sendData(
    (Data){
    .windSpeedOpen  = 0.f,//7.3f,
    .windSpeedClose = 0.f,//9.3f,
    .windSpeedHigh  = 0.f,//10.3f,
    .windSpeedLow   = 0.f,//6.3f,
    .temperature    = bme.temperature,//30.2f,
    .humidity       = bme.humidity,//90.5f,
    .bmp            = bme.bmp,//1019.0f,
    .batteryVolts   = getBatteryVoltage(),
    .windDirection  = 1.f,
  });

  delay(INTERVAL);
}

void sendData(Data data) {
  int ret = sendWifiMessageEnc(HOST, PORT, (uint8_t*)(void*)&data, sizeof(Data), AES_KEY, sizeof(AES_KEY));
}