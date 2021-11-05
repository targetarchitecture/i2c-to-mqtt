#pragma once

#include "messaging.h"
#include "defines.h"
#include "SN8 pins.h"

void sound_setup();
void sound_task(void *pvParameters);
void sound_busy_task(void *pvParameters);

extern void checkI2Cerrors(std::string area);

extern QueueHandle_t Sound_Queue;

