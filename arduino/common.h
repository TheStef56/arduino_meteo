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

#define IF_LED_DEBUG(fn) do {if (MODE == WIFI_LED_DEBUG) {fn;}} while (0)
#define MODE_SELECT_PIN 13

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

typedef enum {
  WIFI_LED_DEBUG,
  WIFI_NO_LED_DEBUG,
  MODE_LENGTH
} Mode;

const char* MODES_TEXT[] = {
  "WIFI_LED_DEBUG",
  "WIFI_NO_LED_DEBUG"
};

const char* MODES_CODE[] = {
  "M1",
  "M2"
};

ArduinoLEDMatrix LED_MATRIX;
Mode MODE;

void ledPrintInit() {
  LED_MATRIX.begin();
}

void ledPrint(const char *text, bool scroll) {
  LED_MATRIX.beginDraw();
  LED_MATRIX.stroke(0xFFFFFFFF);
  LED_MATRIX.textScrollSpeed(50);

  LED_MATRIX.textFont(Font_5x7);
  LED_MATRIX.beginText(1, 1, 0xFFFFFF);
  LED_MATRIX.println(text);
  if (scroll) {
    LED_MATRIX.endText(SCROLL_LEFT);
  } else {
    LED_MATRIX.endText(NO_SCROLL);
  }
  LED_MATRIX.endDraw();
}

void selectMode(unsigned long waitTime) {
  unsigned long startTime = millis();
  Mode mode = WIFI_LED_DEBUG;
  pinMode(MODE_SELECT_PIN, INPUT_PULLUP);

  while (millis() - startTime < waitTime){
    char buf[64] = {0};
    ledPrint(MODES_CODE[(int)mode], false);
    if (!digitalRead(MODE_SELECT_PIN)) {
      mode = (Mode)(((int)mode+1)%(int)MODE_LENGTH);
      ledPrint(MODES_CODE[(int)mode], false);
      delay(200);
      startTime = millis();
    }
  }
  const char buff[64] = {0};
  sprintf((char*)(void*)&buff, "    MODE: %s", MODES_TEXT[(int)mode]);
  ledPrint((char*)(void*)&buff, true);
  MODE = mode;
  pinMode(MODE_SELECT_PIN, INPUT);
}

#endif // COMMON_H