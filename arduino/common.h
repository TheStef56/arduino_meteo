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

#define IF_LED_DEBUG(fn) do {if (SETTINGS | LED_DEBUG) {fn;}} while (0)
#define MODE_SELECT_PIN 13

typedef struct {
  float windSpeedOpen;
  float windSpeedClose;
  float windSpeedHigh;
  float windSpeedLow;
  float windMean;
  float temperature;
  float humidity;
  float bmp;
  float batteryVolts;
  float windDirection;
} Data;

typedef enum {
  NOTHING   = 0,
  LED_DEBUG = 1,
  WIFI      = 2,
  SIM       = 4,
} Settings;

typedef enum {
  WIFI_NO_LED_DEBUG,
  WIFI_LED_DEBUG,
  SIM_NO_LED_DEBUG,
  SIM_LED_DEBUG,
  MODE_LENGTH
} Mode;

const char* MODES_CODE[] = {
  "M1",
  "M2"
};

ArduinoLEDMatrix LED_MATRIX;
Mode MODE;
uint32_t SETTINGS;

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

void makeSettings() {
  switch (MODE) {
    case WIFI_NO_LED_DEBUG:
      SETTINGS = WIFI;
      break;
    case WIFI_LED_DEBUG:
      SETTINGS = WIFI | LED_DEBUG;
      break;
    case SIM_NO_LED_DEBUG:
      SETTINGS = SIM;
      break;
    case SIM_LED_DEBUG:
      SETTINGS = SIM | LED_DEBUG;
      break;
    default:
      break;
  }
}

void selectMode(uint32_t waitTime, uint32_t blinkInterval) {
  uint32_t startTime = millis();
  Mode mode = WIFI_NO_LED_DEBUG;
  pinMode(MODE_SELECT_PIN, INPUT_PULLUP);
  PinStatus prevRead = digitalRead(MODE_SELECT_PIN);
  
  while (millis() - startTime < waitTime + blinkInterval*8){
    uint32_t remainingTime =  waitTime - (millis() - startTime);
    if (
      (remainingTime > blinkInterval*8)                                    ||
      (remainingTime < blinkInterval*8 && remainingTime > blinkInterval*7) ||
      (remainingTime < blinkInterval*6 && remainingTime > blinkInterval*5) ||
      (remainingTime < blinkInterval*4 && remainingTime > blinkInterval*3) ||
      (remainingTime < blinkInterval*2 && remainingTime > blinkInterval*1)
    ) {
      ledPrint(MODES_CODE[(int)mode], false);
    } else {
      ledPrint("    ", false);
    }
    PinStatus lastRead = digitalRead(MODE_SELECT_PIN);
    if (!lastRead && lastRead != prevRead) {
      mode = (Mode)(((int)mode+1)%(int)MODE_LENGTH);
      ledPrint(MODES_CODE[(int)mode], false);
      startTime = millis();
      prevRead = lastRead;
    }
  }
  MODE = mode;
  makeSettings();
  pinMode(MODE_SELECT_PIN, INPUT);
  ledPrint("    ", false);
}

#endif // COMMON_H