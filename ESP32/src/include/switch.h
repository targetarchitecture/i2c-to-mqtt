#pragma once

#include <Wire.h>
//#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"

void switch_setup();
void switch_task(void *pvParameter);
// void switch_deal_with_message(char msg[MAXESP32MESSAGELENGTH]);
// void read_and_send_pin_state();

//extern messageParts processQueueMessage(const std::string msg, const std::string from);
//(char msg[MAXBBCMESSAGELENGTH]);
extern void POST(uint8_t flashes);
extern void checkI2Cerrors(const char *area);

extern SemaphoreHandle_t i2cSemaphore;
