#pragma once

#include <Wire.h>
#include <sstream>
#include <iostream>
#include <Streaming.h>
#include "defines.h"
#include "SN7 pins.h"

void switch_setup();
void switch_task(void *pvParameter);

extern void POST(uint8_t flashes);
extern void checkI2Cerrors(std::string area);

extern SemaphoreHandle_t i2cSemaphore;
