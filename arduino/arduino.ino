#define SERIAL_DEBUG
#include "common.h"

#include "env.h"
#include "wifiMessage.h"
#include "sensors.h"

#define HOST "192.168.0.154"
#define PORT 9000

void setup() {
  IF_SERIAL_DEBUG(
    Serial.begin(9600);
    while (!Serial);
  )
  // setupBme();
  setupWifi();
}

void loop() {
  sendData(
    (Data){
    .windSpeedOpen  = 7.3f,
    .windSpeedClose = 9.3f,
    .windSpeedHigh  = 10.3f,
    .windSpeedLow   = 6.3f,
    .temperature    = 30.2f,
    .humidity       = 90.5f,
    .bmp            = 1019.0f,
    .batteryVolts   = getBatteryVoltage(),
    .windDirection  = 2,
  });

  // IF_SERIAL_DEBUG(Serial.println(getAnemometerVoltage()));
  // IF_SERIAL_DEBUG(Serial.println(getBatteryVoltage()));
  // getBMEdata();
  delay(1000);
}





void sendData(Data data) {
  int ret = sendWifiMessageEnc(HOST, PORT, (uint8_t*)(void*)&data, sizeof(Data), AES_KEY, sizeof(AES_KEY));
}