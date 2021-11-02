#pragma once

#include <string>
#include "defines.h"
#include "SN8 pins.h"
#include "string.h"
#include <iostream>
#include <string>
#include <vector>
#include <Streaming.h>
#include <regex>
#include <stdint.h>
#include <Stream.h>
#include <sstream>

typedef struct
{
    char identifier[20];
    uint32_t value1;
    uint32_t value2;
    uint32_t value3;
    uint32_t value4;
    uint32_t value5;
    uint32_t value6;
    uint32_t value7;
    char part1[100];
} messageParts;

messageParts processQueueMessage(std::string msg);

extern QueueHandle_t Sound_Queue;
extern QueueHandle_t DAC_Queue;
extern QueueHandle_t Light_Queue;
extern QueueHandle_t Movement_Queue;

extern volatile int32_t encoder1Count;
extern volatile int32_t encoder2Count;
extern volatile byte switchArray[16];
extern volatile int touchArray[12];

extern void touch_deal_with_message(messageParts message);
void sendToMicrobit(std::string msg);