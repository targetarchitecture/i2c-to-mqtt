#pragma once

#include <Wire.h>
#include "messaging.h"
#include "defines.h"
#include "pins.h"
#include "Adafruit_MPR121.h"
#include <Preferences.h>
//#include <Streaming.h>
// #include <Stream.h>
// #include <sstream>

void touch_setup();
void touch_task(void *pvParameter);
void touch_deal_with_message(messageParts message);
uint16_t readAndSetTouchArray();

extern void POST(uint8_t flashes);
extern void checkI2Cerrors(std::string area);

extern SemaphoreHandle_t i2cSemaphore;
extern Preferences preferences;