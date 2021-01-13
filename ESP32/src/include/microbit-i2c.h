#ifndef microbit_i2c_h
#define microbit_i2c_h

#include <Wire.h>
#include <WireSlave.h>
#include <WirePacker.h>
#include "driver/uart.h"
#include "defines.h"
#include "SN7 pins.h"

#define I2C_SLAVE_ADDR 4

//extern QueueHandle_t Message_Queue;
extern QueueHandle_t Microbit_Transmit_Queue;
extern QueueHandle_t Microbit_Receive_Queue;

void microbit_i2c_setup();
// void microbit_receive_task(void *pvParameters);
// void microbit_transmit_task(void *pvParameters);
void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
//void microbit_receive_task(void *pvParameters);

//extern void checkI2Cerrors(const char *area);

void receiveEvent(int howMany); //when BBC Microbit calls i2cWriteBuffer
void requestEvent();  //when BBC Microbit call i2cReadBuffer
void IRAM_ATTR handleBBCi2CInterupt();
void i2c_rx_task(void *pvParameter);


#endif
