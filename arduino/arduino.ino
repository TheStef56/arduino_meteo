#define SERIAL_DEBUG
#include "common.h"

#include "env.h"
#include "wifiMessage.h"

#define ANEMOMETER_ANALOG 0
#define HOST "192.168.0.154"
#define PORT 9000

#include <Adafruit_BME280.h>

Adafruit_BME280 bme; // I2C interface

void setup() {
  IF_SERIAL_DEBUG(
    Serial.begin(9600);
    while (!Serial);
  )
  // setupBme();
}

void loop() {
  sendMockData();
  // IF_SERIAL_DEBUG(Serial.println(getAnemometerVoltage()));
  // getBMEdata();
  delay(1000);
}

void getBMEdata() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" °C");

  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}

void setupBme() {
  if (!bme.begin(0x76)) {  // Try 0x76 or 0x77 depending on your module
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

float getAnemometerVoltage() {
  int value = analogRead(ANEMOMETER_ANALOG);
  return value*5.0f/1023.0f;
}

void sendMockData() {
  Data data = {
    .windSpeedOpen  = 7.3f,
    .windSpeedClose = 9.3f,
    .windSpeedHigh  = 10.3f,
    .windSpeedLow   = 6.3f,
    .temperature    = 30.2f,
    .humidity       = 90.5f,
    .bmp            = 1019.0f,
    .batteryVolts   = 12.8f,
    .windDirection  = 2,
  };

  sendWifiMessageEnc(HOST, PORT, (uint8_t*)(void*)&data, sizeof(Data), AES_KEY, sizeof(AES_KEY));
}