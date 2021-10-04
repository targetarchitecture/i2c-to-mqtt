#pragma once

#include <Wire.h>
#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"

void light_setup();
void light_task(void *pvParameter);
void stopLight(long pin);

extern void POST(uint8_t flashes);
extern void checkI2Cerrors(std::string area);

extern QueueHandle_t Light_Queue;
extern SemaphoreHandle_t i2cSemaphore;