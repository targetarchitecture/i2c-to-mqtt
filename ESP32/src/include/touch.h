#ifndef touch_h
#define touch_h

#include <Wire.h>
#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"

void touch_setup();
void touch_task(void *pvParameter);
void IRAM_ATTR handleTouchInterupt();
void IRAM_ATTR handleTouchInteruptLOW();
void IRAM_ATTR handleTouchInteruptHIGH();

extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern messageParts processQueueMessage(const std::string msg, const std::string from);
extern void POST(uint8_t flashes);
extern void checkI2Cerrors(const char *area);

extern SemaphoreHandle_t i2cSemaphore;


#endif