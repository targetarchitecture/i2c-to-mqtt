#pragma once

#include "messaging.h"
#include "defines.h"
#include "SN9 pins.h"
#include "DFRobotDFPlayerMini.h"

void sound_setup();
void sound_task(void *pvParameters);
void sound_busy_taskV2(void *pvParameters);
void sound_busy_taskV1(void *pvParameters);

void IRAM_ATTR handleFallingInterupt();
void IRAM_ATTR handleRisingInterupt();

extern void checkI2Cerrors(std::string area);

extern QueueHandle_t Sound_Queue;

