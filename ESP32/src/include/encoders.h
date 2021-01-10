#ifndef encoder_h
#define encoder_h

#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"

void encoders_setup();
void encoders_task(void *pvParameter);
void encoders_deal_with_message(char msg[MAXMESSAGELENGTH]);

extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern messageParts processQueueMessage(const std::string msg, const std::string from) ;
extern void checkI2Cerrors(const char *area);

#endif