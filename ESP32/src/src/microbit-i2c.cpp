#include <Arduino.h>
#include "microbit-i2c.h"

void microbit_setup()
{
 
}

void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH])
{
    //Serial.print("sendToMicrobit [msg]:");
    //Serial.println(msg);

    //the queue needs to work with a copy
    char queuedMsg[MAXBBCMESSAGELENGTH];
    strcpy(queuedMsg, msg);

    //xQueueSend(Microbit_Transmit_Queue, &queuedMsg, portMAX_DELAY);
}

