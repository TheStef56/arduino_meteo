#ifndef COMMON_H
#define COMMON_H

#include <WiFiS3.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>


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

ArduinoLEDMatrix LED_MATRIX;

void ledPrintInit() {
    LED_MATRIX.begin();
}

void ledPrint(const char *text) {
    LED_MATRIX.beginDraw();
    LED_MATRIX.stroke(0xFFFFFFFF);
    LED_MATRIX.textScrollSpeed(50);

    LED_MATRIX.textFont(Font_5x7);
    LED_MATRIX.beginText(0, 1, 0xFFFFFF);
    LED_MATRIX.println(text);
    LED_MATRIX.endText(SCROLL_LEFT);

    LED_MATRIX.endDraw();
}

#endif // COMMON_H