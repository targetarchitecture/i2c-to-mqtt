#ifndef music_h
#define music_h

#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"

void music_setup();
void music_task(void *pvParameters);
void music_busy_task(void *pvParameters);

extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern messageParts processQueueMessage(const std::string msg, const std::string from) ;
extern void checkI2Cerrors(const char *area);

extern QueueHandle_t Music_Queue;


#endif
