#pragma once

#include "driver/uart.h"
#include "defines.h"
#include "pins.h"
#include "messaging.h"

extern QueueHandle_t Microbit_Transmit_Queue;
extern QueueHandle_t Microbit_Receive_Queue;

void microbit_setup();
void microbit_receive_task(void *pvParameters);
void microbit_transmit_task(void *pvParameters);
void microbit_receive_task(void *pvParameters);

extern void checkI2Cerrors(const char *area);
extern void dealWithMessage(std::string message);
