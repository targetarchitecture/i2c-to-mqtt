#ifndef ADC_h
#define ADC_h

#include "defines.h"
#include "SN4 pins.h"

void ADC_setup();
void ADC_task(void *pvParameter);

extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern SemaphoreHandle_t i2cSemaphore;

#endif