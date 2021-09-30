#pragma once

#include <Wire.h>
#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"

void light_setup();
void light_task(void *pvParameter);
void stopLight(long pin);

//extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern messageParts processQueueMessage(const std::string msg, const std::string from);
extern void POST(uint8_t flashes);
extern void checkI2Cerrors(const char *area);

extern QueueHandle_t Light_Queue;
extern SemaphoreHandle_t i2cSemaphore;

//extern char TXtoBBCmessage[MAXBBCMESSAGELENGTH];
//extern char RXfromBBCmessage[MAXESP32MESSAGELENGTH];