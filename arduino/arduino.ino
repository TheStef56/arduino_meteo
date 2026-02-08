#define SERIAL_DEBUG
#include "ctx.h"
#include "env.h"
#include "wifiMessage.h"
#include "simMessage.h"
#include "sensors.h"

size_t count = 0;
float windMean = 0.f;
float windOpen = 0.f;
float windMax = 0.f;
float windMin = 9999.f;

void (*sendData)(WindData data);

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
  
  if (SETTINGS & SIM) {
    Serial1.begin(115200);
    sendData = sendSimData;
  }
  if (SETTINGS & WIFI) {
    setupWifi();
    sendData = sendWifiData;
  }
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
  sendSimMessageEnc(REMOTE_HOST, REMOTE_PORT, (uint8_t*)(void*)&data, sizeof(WindData), AES_KEY, sizeof(AES_KEY));
}

void sendWifiData(WindData data) {
  sendWifiMessageEnc(WIFI_HOST, WIFI_PORT, (uint8_t*)(void*)&data, sizeof(WindData), AES_KEY, sizeof(AES_KEY));
}