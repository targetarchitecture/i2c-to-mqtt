#pragma once

#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"

void DAC_setup();
void DAC_task(void *pvParameter);

//extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
//extern messageParts processQueueMessage(const std::string msg, const std::string from);
extern void checkI2Cerrors(std::string area);


extern QueueHandle_t DAC_Queue;

// extern char TXtoBBCmessage[MAXBBCMESSAGELENGTH];
// extern char RXfromBBCmessage[MAXESP32MESSAGELENGTH];