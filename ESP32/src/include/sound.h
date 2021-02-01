#pragma once

#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"

void sound_setup();
void sound_task(void *pvParameters);
void sound_busy_task(void *pvParameters);
void IRAM_ATTR handleSoundInterupt();

extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern messageParts processQueueMessage(const std::string msg, const std::string from) ;
extern void checkI2Cerrors(const char *area);

extern QueueHandle_t Sound_Queue;

