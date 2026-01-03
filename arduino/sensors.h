#ifndef SENSORS_H
#define SENSORS_H
#include "SoftI2C_BME280.h"

#define BATTERY_ANALOG A0
#define WIND_DIR_ANALOG A5
#define ANEMOMETER_ANALOG A4

#define READS 100

SoftI2C_BME280 BME(8); // delayUs = 8 (safe);

typedef struct {
  float temperature;
  float humidity;
  float bmp;
} BMEData;

float denoisedAnalogRead(int pin) {
  int minVal = 1024, maxVal = -1;
  long sum = 0;
  for (int i = 0; i < READS; ++i) {
    int v = analogRead(pin);
    sum += v;
    if (v < minVal) minVal = v;
    if (v > maxVal) maxVal = v;
    delayMicroseconds(20); // let ADC sample capacitor settle
  }
  sum -= minVal;
  sum -= maxVal; // remove one highest and one lowest
  float avg = sum / float(READS - 2);
  return avg;
}

#define getBatteryVoltage() denoisedAnalogRead(BATTERY_ANALOG)*50/1023.0f
#define getWindDirectionDegrees() denoisedAnalogRead(WIND_DIR_ANALOG)*360/1023.0f
#define getWindSpeedKmH() denoisedAnalogRead(ANEMOMETER_ANALOG)*252/1023.0f

void setupBme() {
  if (!BME.begin(D2, D3, 0x76)) {  // Try 0x76 or 0x77 depending on the module
    IF_SERIAL_DEBUG(Serial.println("Could not find a valid BME280 sensor!"));
    while (1) {
      ledPrint("    BME280 setup error!", true);
    };
  }
}

BMEData getBMEdata() {
  BMEData data;
  BME.readAll(
    data.temperature,
    data.bmp,
    data.humidity
  );
  return data;
}

#endif // SENSORS_H