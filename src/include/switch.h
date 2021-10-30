#pragma once

#include <Wire.h>
#include <sstream>
#include <iostream>
#include <Streaming.h>
#include "defines.h"
#include "SN8 pins.h"
#include <string>
#include <vector>
#include <array>
#include "messaging.h"

void switch_setup();
void switch_task(void *pvParameter);
std::string readAndSetSwitchArray();

extern void POST(uint8_t flashes);
extern void checkI2Cerrors(std::string area);

extern SemaphoreHandle_t i2cSemaphore;
