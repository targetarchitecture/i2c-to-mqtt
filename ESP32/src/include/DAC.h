#ifndef DAC_h
#define DAC_h

#include "messageParts.h"
#include "defines.h"
#include "SN4 pins.h"

void DAC_setup();
void DAC_task(void *pvParameter);

extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern messageParts processQueueMessage(const std::string msg, const std::string from) ;
extern void checkI2Cerrors(const char *area);

extern QueueHandle_t DAC_Queue;


#endif