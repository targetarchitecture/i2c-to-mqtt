#ifndef microbit_h
#define microbit_h

#include "driver/uart.h"
#include "defines.h"
#include "SN4 pins.h"

#define BBC_UART_NUM UART_NUM_2

extern QueueHandle_t Message_Queue;
extern QueueHandle_t Microbit_Transmit_Queue;
extern QueueHandle_t Microbit_Receive_Queue;

void microbit_setup();
void microbit_receive_task(void *pvParameters);
void microbit_transmit_task(void *pvParameters);
void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
void microbit_receive_task(void *pvParameters);

extern void checkI2Cerrors(const char *area);

#endif
