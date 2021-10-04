#pragma once

#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"
#include <Streaming.h>
//#include <sstream>

void DAC_setup();
void DAC_task(void *pvParameter);

extern void checkI2Cerrors(std::string area);
extern QueueHandle_t DAC_Queue;
