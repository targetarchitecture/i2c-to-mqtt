#ifndef light_h
#define light_h

#include <Wire.h>
#include "messageParts.h"
#include "defines.h"
#include "SN4 pins.h"

void light_setup();
void light_task(void *pvParameter);
void stopLight(long pin);

extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern messageParts processQueueMessage(const std::string msg, const std::string from);
extern void POST(uint8_t flashes);

extern QueueHandle_t Light_Queue;
extern SemaphoreHandle_t i2cSemaphore;

#endif