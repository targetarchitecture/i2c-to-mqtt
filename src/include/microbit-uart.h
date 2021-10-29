#pragma once

#include "driver/uart.h"
#include "defines.h"
#include "SN8 pins.h"
#include <Streaming.h>
#include "string.h"
#include <iostream>
#include <string>
#include <vector>
#include "messaging.h"
#include <regex>
#include <stdint.h>
#include <Stream.h>
#include <sstream>

extern QueueHandle_t Microbit_Transmit_Queue;
extern QueueHandle_t Microbit_Receive_Queue;

void microbit_setup();
void microbit_receive_task(void *pvParameters);
void microbit_transmit_task(void *pvParameters);
void microbit_receive_task(void *pvParameters);

extern void checkI2Cerrors(const char *area);
extern void dealWithMessage(std::string message);
