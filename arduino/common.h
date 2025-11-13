#ifndef COMMON_H
#define COMMON_H

#include <WiFiS3.h>

#ifdef SERIAL_DEBUG
  #define IF_SERIAL_DEBUG(serial) serial
#else
  #define IF_SERIAL_DEBUG(serial)
#endif

typedef struct {
  float windSpeedOpen;
  float windSpeedClose;
  float windSpeedHigh;
  float windSpeedLow;
  float temperature;
  float humidity;
  float bmp;
  float batteryVolts;
  byte windDirection;
} Data;

#endif // COMMON_H