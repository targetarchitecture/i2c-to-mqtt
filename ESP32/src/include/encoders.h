#pragma once

#include "messageParts.h"
#include "defines.h"
#include "SN7 pins.h"

void encoders_setup();
void encoders_task(void *pvParameter);
//void encoders_deal_with_message(char msg[MAXESP32MESSAGELENGTH]);

//extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
//extern messageParts processQueueMessage( std::string msg,  std::string from) ;
//extern void checkI2Cerrors(std::string area);
