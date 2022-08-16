#pragma once

#include "messaging.h"
#include "defines.h"
//#include "pins.h"
//#include <Streaming.h>

void DAC_setup();
void DAC_task(void *pvParameter);

extern void checkI2Cerrors(std::string area);
extern QueueHandle_t DAC_Queue;
