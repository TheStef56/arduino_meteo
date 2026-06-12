#ifndef COMMON_H
#define COMMON_H
#include <WiFiS3.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>
#include <EEPROM.h>

int WIND_MEASURING_INTERVAL = 5*1000;    // 5 sec
int DATA_SENDING_INTERVAL   = 5*60*1000; // 5 min

#ifdef SERIAL_DEBUG
  #define IF_SERIAL_DEBUG(fn) do {if (SETTINGS & DEBUG) {fn;}} while (0)
#else
  #define IF_SERIAL_DEBUG(serial)
#endif

#define IF_LED_DEBUG(fn) do {if (SETTINGS & LED_DEBUG) {fn;}} while (0)
#define MODE_SELECT_PIN 13

#define ledPrintInit()   LED_MATRIX.begin()
#define ledPrintDeInit() LED_MATRIX.end()

#include "proto.h"

const uint32_t MAGIC = 0xCAFEBABE;

typedef struct {
  uint32_t magic;
  int mode;
} EEPROM_Config;

typedef enum {
  NOTHING         = 0,
  LED_DEBUG       = 1,
  DEBUG           = 2,
  WIFI            = 4,
  SIM             = 8,
  SIM_NO_SHUTDOWN = 16
} Settings;

typedef enum {
  WIFI_NO_LED_DEBUG,
  WIFI_DEBUG,
  WIFI_LED_DEBUG,
  SIM_NO_LED_DEBUG,
  SIM_NO_LED_DEBUG_NO_SHUTDOWN,
  SIM_DEBUG,
  SIM_DEBUG_NO_SHUTDOWN,
  SIM_LED_DEBUG,
  MODE_LENGTH
} Mode;

const char* MODES_CODE[] = {
  "WN",
  "WD",
  "WL",
  "SN",
  "SNN",
  "SD",
  "SDN",
  "SL",
};

ArduinoLEDMatrix LED_MATRIX;
Mode MODE;
uint32_t SETTINGS;

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
    case WIFI_DEBUG:
      SETTINGS = WIFI | DEBUG;
      break;
    case WIFI_LED_DEBUG:
      SETTINGS = WIFI | DEBUG | LED_DEBUG;
      break;
    case SIM_NO_LED_DEBUG:
      SETTINGS = SIM;
      break;
    case SIM_NO_LED_DEBUG_NO_SHUTDOWN:
      SETTINGS = SIM | SIM_NO_SHUTDOWN;
      break;
    case SIM_DEBUG:
      SETTINGS = SIM | DEBUG;
      break;
    case SIM_DEBUG_NO_SHUTDOWN:
      SETTINGS = SIM | DEBUG | SIM_NO_SHUTDOWN;
      break;
    case SIM_LED_DEBUG:
      SETTINGS = SIM | DEBUG | LED_DEBUG;
      break;
    default:
      break;
  }
  if (SETTINGS & DEBUG) {
    WIND_MEASURING_INTERVAL = 200;
    DATA_SENDING_INTERVAL   = 1000;
  }
  if (!(SETTINGS & LED_DEBUG)) {
    ledPrintDeInit();
  }
}

void selectMode(uint32_t waitTime, uint32_t blinkInterval) {
  uint32_t startTime = millis();
  EEPROM_Config stored_config;
  EEPROM.get(0, stored_config);
  Mode mode;
  if (stored_config.magic != MAGIC | digitalRead(MODE_SELECT_PIN) == LOW) {    // if config has never been stored to EEPROM or we want to force default by pressing button on startup
    mode = SIM_NO_LED_DEBUG_NO_SHUTDOWN;                                       // default mode
  } else {
    mode = (Mode)stored_config.mode;                                           // stored mode
  }

  while (digitalRead(MODE_SELECT_PIN) == LOW);                                 // if we pressed button on startup, let's wait until it's not pressed to avoid cycling
  delay(100);                                                                  // debounce

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
    }
    prevRead = lastRead;
  }
  MODE = mode;
  makeSettings();
  pinMode(MODE_SELECT_PIN, INPUT);
  ledPrint("    ", false);
  stored_config.magic = MAGIC;
  stored_config.mode = MODE;
  EEPROM.put(0, stored_config);
}

#endif // COMMON_H