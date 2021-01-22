/* 
Rainbow Sparkle Unicorn - SN7
*/
#include <Arduino.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <vector>
#include <sstream>
#include <iostream>

#include "globals.h"
#include "microbit-i2c.h"
#include "sound.h"
#include "encoders.h"
#include "routing.h"
#include "touch.h"
#include "DAC.h"
#include "ADC.h"
#include "light.h"
#include "switch.h"
#include "movement.h"
#include "MQTT.h"

QueueHandle_t Microbit_Transmit_Queue; //Queue to send messages to the Microbit
QueueHandle_t Microbit_Receive_Queue;  //Queue to recieve the messages from the Microbit
QueueHandle_t Sound_Queue;             //Queue to store all of the DFPlayer commands from the Microbit
QueueHandle_t DAC_Queue;
QueueHandle_t Light_Queue;
QueueHandle_t ADC_Queue;
QueueHandle_t Movement_Queue;
QueueHandle_t MQTT_Queue;

extern PubSubClient MQTTClient;
void checkI2Cerrors(const char *area);

void setup()
{
  //Set UART log level
  esp_log_level_set("SN7", ESP_LOG_VERBOSE);

  //stop bluetooth
  btStop();

  //start i2c
  Wire.begin(SDA, SCL);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("");
  Serial.println("");

  //create i2c Semaphore , and set to useable
  i2cSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(i2cSemaphore);

  //set up the main queues
  char TXtoBBCmessage[MAXBBCMESSAGELENGTH];
  char RXfromBBCmessage[MAXESP32MESSAGELENGTH];
  //char MAXUSBMessage[UARTMESSAGELENGTH];

  Microbit_Transmit_Queue = xQueueCreate(50, sizeof(TXtoBBCmessage));
  Microbit_Receive_Queue = xQueueCreate(50, sizeof(RXfromBBCmessage));

  Sound_Queue = xQueueCreate(50, sizeof(RXfromBBCmessage));
  DAC_Queue = xQueueCreate(50, sizeof(RXfromBBCmessage));
  Light_Queue = xQueueCreate(50, sizeof(RXfromBBCmessage));
  Movement_Queue = xQueueCreate(50, sizeof(RXfromBBCmessage));
  MQTT_Queue = xQueueCreate(50, sizeof(RXfromBBCmessage));

  //get wifi going first as this seems to be problematic
  MQTT_setup();

  //call the feature setup methods
  sound_setup();

  touch_setup();

  encoders_setup();

  DAC_setup();

  ADC_setup();

  light_setup();

  switch_setup();

  movement_setup();

  routing_setup();

  microbit_i2c_setup();

  // Serial.print("SN7 completed in ");
  // Serial.println(millis());
}

void loop()
{
  delay(1000);
}

messageParts processQueueMessage(const std::string msg, const std::string from)
{
  //std::vector<std::string> strings;
  std::istringstream f(msg);
  std::string part;

  messageParts mParts = {};
  strcpy(mParts.fullMessage, msg.c_str());
  int index = 0;

  Serial.println(msg.c_str());

  while (std::getline(f, part, ','))
  {
    //strings.push_back(s);

    Serial.println(part.c_str());

    if (index == 0)
    {
      strcpy(mParts.identifier, part.c_str());
    }
    if (index == 1)
    {
      strcpy(mParts.value1, part.c_str());
    }
    if (index == 2)
    {
      strcpy(mParts.value2, part.c_str());
    }
    if (index == 3)
    {
      strcpy(mParts.value3, part.c_str());
    }
    if (index == 4)
    {
      strcpy(mParts.value4, part.c_str());
    }
    if (index == 5)
    {
      strcpy(mParts.value5, part.c_str());
    }
    if (index == 6)
    {
      strcpy(mParts.value6, part.c_str());
    }
    if (index == 7)
    {
      strcpy(mParts.value7, part.c_str());
    }

    index++;
  }

  return mParts;
}

//TODO: check that the the new method works before deleting this one.
messageParts processQueueMessageV1(const std::string msg, const std::string from)
{
  //Serial.printf("processQueueMessage (%s): %s\n", from.c_str(), msg.c_str());

  messageParts mParts = {};
  strcpy(mParts.fullMessage, msg.c_str());

  try
  {
    //https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    std::string delim = ",";
    int index = 0;
    auto start = 0U;
    auto end = msg.find(delim);

    while (end != std::string::npos)
    {
      if (index == 0)
      {
        strcpy(mParts.identifier, msg.substr(start, end - start).c_str());
      }
      if (index == 1)
      {
        strcpy(mParts.value1, msg.substr(start, end - start).c_str());
      }
      if (index == 2)
      {
        strcpy(mParts.value2, msg.substr(start, end - start).c_str());
      }
      if (index == 3)
      {
        strcpy(mParts.value3, msg.substr(start, end - start).c_str());
      }
      if (index == 4)
      {
        strcpy(mParts.value4, msg.substr(start, end - start).c_str());
      }
      if (index == 5)
      {
        strcpy(mParts.value5, msg.substr(start, end - start).c_str());
      }
      if (index == 6)
      {
        strcpy(mParts.value6, msg.substr(start, end - start).c_str());
      }
      if (index == 7)
      {
        strcpy(mParts.value7, msg.substr(start, end - start).c_str());
      }

      start = end + delim.length();
      end = msg.find(delim, start);

      index++;
    }

    //it's a bit crap to repeat the logic - but it works
    if (index == 0)
    {
      strcpy(mParts.identifier, msg.substr(start, end - start).c_str());
    }
    if (index == 1)
    {
      strcpy(mParts.value1, msg.substr(start, end - start).c_str());
    }
    if (index == 2)
    {
      strcpy(mParts.value2, msg.substr(start, end - start).c_str());
    }
    if (index == 3)
    {
      strcpy(mParts.value3, msg.substr(start, end - start).c_str());
    }
    if (index == 4)
    {
      strcpy(mParts.value4, msg.substr(start, end - start).c_str());
    }
    if (index == 5)
    {
      strcpy(mParts.value5, msg.substr(start, end - start).c_str());
    }
    if (index == 6)
    {
      strcpy(mParts.value6, msg.substr(start, end - start).c_str());
    }
    if (index == 7)
    {
      strcpy(mParts.value7, msg.substr(start, end - start).c_str());
    }

    // Serial.print("identifier:");
    // Serial.println(mParts.identifier);
    // Serial.print("value1:");
    // Serial.println(mParts.value1);
    // Serial.print("value2:");
    // Serial.println(mParts.value2);
    // Serial.print("value3:");
    // Serial.println(mParts.value3);
    // Serial.print("value4:");
    // Serial.println(mParts.value4);
    // Serial.print("value5:");
    // Serial.println(mParts.value5);
    // Serial.print("value6:");
    // Serial.println(mParts.value6);
    // Serial.print("value7:");
    // Serial.println(mParts.value7);
    // Serial.print("fullMessage:");
    // Serial.println(mParts.fullMessage);
  }
  catch (const std::exception &e)
  {
    Serial.printf("\n\n\nEXCEPTION: %s \n\n\n", e.what());
  }

  return mParts;
}

void POST(uint8_t flashes)
{
  //TODO: debate which tasks need stopping?
  vTaskDelete(ADCTask);

  pinMode(ONBOARDLED, OUTPUT);

  uint32_t speed = 150;

  for (;;)
  {
    for (size_t i = 0; i < flashes; i++)
    {
      digitalWrite(ONBOARDLED, HIGH);
      delay(speed);
      digitalWrite(ONBOARDLED, LOW);
      delay(speed);
    }
    delay(1000);
  }
}

void checkI2Cerrors(const char *area)
{
  if (Wire.lastError() != 0)
  {
    Serial.printf("i2C error @ %s: %s \n", area, Wire.getErrorText(Wire.lastError()));

    //TODO: Check to see if this is still needed
    // Wire.clearWriteError();
  }
}
