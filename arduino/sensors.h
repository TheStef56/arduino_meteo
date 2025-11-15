#ifndef SENSORS_H
#define SENSORS_H

#define ANEMOMETER_ANALOG 0
#define BATTERY_ANALOG 1

#include <Adafruit_BME280.h>
Adafruit_BME280 bme; // I2C interface

typedef struct {
  float temperature;
  float pressure;
  float humidity;
} BMEData;

float getBatteryVoltage() {
  int value = analogRead(BATTERY_ANALOG);
  return value*50.f/1023.0f;
}

float getAnemometerVoltage() {
  int value = analogRead(ANEMOMETER_ANALOG);
  return value*5.0f/1023.0f;
}

void setupBme() {
  if (!bme.begin(0x76)) {  // Try 0x76 or 0x77 depending on the module
    IF_SERIAL_DEBUG(Serial.println("Could not find a valid BME280 sensor!"));
    while (1);
  }
}

BMEData getBMEdata() {
  BMEData data;
  data.temperature = bme.readTemperature();       // °C
  data.pressure =  (bme.readPressure() / 100.0F); // hPa
  data.humidity = bme.readHumidity();             // %
  return data;
}

#endif // SENSORS_H