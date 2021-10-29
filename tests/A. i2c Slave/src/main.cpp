#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <Wire.h>
#include <WireSlave.h>
#include <SN7 pins.h>
#include <WirePacker.h>
#include "driver/uart.h"
#include "SN7 pins.h"

TaskHandle_t SwitchTask;
QueueHandle_t Microbit_Transmit_Queue; //Queue to send messages to the Microbit

void receiveEvent(int howMany);
void requestEvent();
void IRAM_ATTR myisr();
void i2c_rx_task(void *pvParameter);

#define I2C_SLAVE_ADDR 4
#define MAXBBCMESSAGELENGTH 20 //This is the length of the message sent to the BBC Microbit (was 10 - needs to be 18 for IP address)

void setup()
{
  Serial.begin(115200);

  pinMode(2, OUTPUT);

  Wire.begin(SDA, SCL); //I2C bus

  bool success = WireSlave1.begin(MICROBIT_SDA, MICROBIT_SCL, I2C_SLAVE_ADDR);

  if (!success)
  {
    Serial.println("I2C slave init failed");
    while (1)
      delay(100);
  }

  WireSlave1.onReceive(receiveEvent);
  WireSlave1.onRequest(requestEvent);

  pinMode(BBC_INT, INPUT_PULLUP);
  attachInterrupt(BBC_INT, myisr, RISING);

  xTaskCreatePinnedToCore(
      i2c_rx_task,     /* Task function. */
      "i2c RX Task",   /* name of task. */
      8500,            /* Stack size of task (uxTaskGetStackHighWaterMark: 8200) */
      NULL,            /* parameter of the task */
      1,               /* priority of the task */
      &SwitchTask, 1); /* Task handle to keep track of created task */

  //add test messages
  char TXtoBBCmessage[MAXBBCMESSAGELENGTH ];
  Microbit_Transmit_Queue = xQueueCreate(50, sizeof(TXtoBBCmessage));
}

void IRAM_ATTR myisr()
{
  xTaskNotify(SwitchTask, 0, eSetValueWithoutOverwrite);
}

void i2c_rx_task(void *pvParameter)
{
  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  Serial.print("i2c_rx_task uxTaskGetStackHighWaterMark:");
  Serial.println(uxHighWaterMark);

  uint32_t ulNotifiedValue = 0;
  BaseType_t xResult;

  for (;;)
  {
    //10/12/20 - Just wait around to see if we get hailed to send
    xResult = xTaskNotifyWait(0X00, 0x00, &ulNotifiedValue, portMAX_DELAY);

    delay(1);

    WireSlave1.update();
  }
}

void loop()
{
  delay(1000);

  char TXtoBBCmessage[MAXBBCMESSAGELENGTH ];

  sprintf(TXtoBBCmessage, "G1,%s", "192.168.134.123");
  xQueueSend(Microbit_Transmit_Queue, &TXtoBBCmessage, portMAX_DELAY);
}

// function that executes whenever a complete and valid packet
// is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  while (1 < WireSlave1.available()) // loop through all but the last byte
  {
    char c = WireSlave1.read(); // receive byte as a character
    //Serial.print(c);            // print the character
  }

  char c = WireSlave1.read(); // receive byte as an integer
  //Serial.println(c);
}

// void requestEvent()
// {
//   Serial.println(millis());
//    WireSlave1.printf("hello to your family @ %i", millis());
// }

void requestEvent()
{
  char msg[MAXBBCMESSAGELENGTH] = {0};

  //wait for new BBC command in the queue
  if (xQueueReceive(Microbit_Transmit_Queue, &msg, 0))
  {
    Serial.print("sent: ");
    Serial.println(msg);

    Serial.print("strlen: ");
    Serial.println(strlen(msg));

    WireSlave1.print(msg);
  }
}