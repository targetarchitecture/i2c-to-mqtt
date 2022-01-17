#pragma once

#include <Wire.h>
#include "defines.h"
#include "pins.h"
#include "messaging.h"

void switch_setup();
void switch_task(void *pvParameter);
std::string readAndSetSwitchArray();

extern void POST(uint8_t flashes);
extern void checkI2Cerrors(std::string area);

extern SemaphoreHandle_t i2cSemaphore;
