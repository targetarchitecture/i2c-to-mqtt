#pragma once

#include <Wire.h>
#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"

void touch_setup();
void touch_task(void *pvParameter);
void IRAM_ATTR handleTouchInterupt();
void touch_deal_with_message(const char *msg);

extern messageParts processQueueMessage( std::string msg,  std::string from);
extern void POST(uint8_t flashes);
extern void checkI2Cerrors(std::string area);

extern SemaphoreHandle_t i2cSemaphore;
