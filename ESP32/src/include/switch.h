#ifndef switch_h
#define switch_h

#include <Wire.h>
#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"

void switch_setup();
void switch_task(void *pvParameter);

extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern void POST(uint8_t flashes);
extern void checkI2Cerrors(const char *area);

extern SemaphoreHandle_t i2cSemaphore;

#endif