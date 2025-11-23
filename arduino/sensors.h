#include <Adafruit_BME280.h>

#ifndef SENSORS_H
#define SENSORS_H

#define ANEMOMETER_ANALOG 0
#define BATTERY_ANALOG 1

#define READS 100

Adafruit_BME280 BME; // I2C interface

typedef struct {
  float temperature;
  float humidity;
  float bmp;
} BMEData;

float getBatteryVoltage() {
  int minVal = 1024, maxVal = -1;
  long sum = 0;
  for (int i = 0; i < READS; ++i) {
    int v = analogRead(BATTERY_ANALOG);
    sum += v;
    if (v < minVal) minVal = v;
    if (v > maxVal) maxVal = v;
    delayMicroseconds(20); // let ADC sample capacitor settle
  }
  sum -= minVal;
  sum -= maxVal; // remove one highest and one lowest
  float avg = sum / float(READS - 2);
  return avg * 50.0f / 1023.0f;
}

float getAnemometerVoltage() {
  int value = analogRead(ANEMOMETER_ANALOG);
  return value*5.0f/1023.0f;
}

void setupBme() {
  if (!BME.begin(0x76)) {  // Try 0x76 or 0x77 depending on the module
    IF_SERIAL_DEBUG(Serial.println("Could not find a valid BME280 sensor!"));
    while (1) {
      ledPrint("    BME280 setup error!", true);
    };
  }
}

BMEData getBMEdata() {
  BMEData data;
  data.temperature = BME.readTemperature();       // °C
  data.humidity = BME.readHumidity();             // %
  data.bmp =  (BME.readPressure() / 100.0F); // hPa
  return data;
}

#endif // SENSORS_H