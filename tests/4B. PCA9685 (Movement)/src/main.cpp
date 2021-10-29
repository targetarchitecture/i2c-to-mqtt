
#include <Arduino.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <vector>
#include <sstream>
#include <iostream>
#include <Adafruit_PWMServoDriver.h>
#include "globals.h"
#include "movement.h"

QueueHandle_t Movement_Queue;

void setup()
{
  Wire.begin(SDA, SCL); //I2C bus

  Serial.begin(115200);

  delay(100);

  Serial.println("\n\nPCA9685");

  //create i2c Semaphore , and set to useable
  i2cSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(i2cSemaphore);

  Movement_Queue = xQueueCreate(50, sizeof(RXfromBBCmessage));

  movement_setup();

  char cmd[MAXBBCMESSAGELENGTH];

  sprintf(cmd, "V2,8,110,100,500");
  xQueueSend(Movement_Queue, &cmd, portMAX_DELAY);

  // delay(500);
  // sprintf(cmd, "V2,0,110,100,500");
  // xQueueSend(Movement_Queue, &cmd, portMAX_DELAY);

  // delay(500);
  // sprintf(cmd, "V2,0,180,100,500");
  // xQueueSend(Movement_Queue, &cmd, portMAX_DELAY);

  // delay(500);
  // sprintf(cmd, "V2,0,110,100,500");
  // xQueueSend(Movement_Queue, &cmd, portMAX_DELAY);

  ///Serial.println("\n\neasing:\n");
}

void loop()
{
  char cmd[MAXBBCMESSAGELENGTH];

  sprintf(cmd, "V3,8,90,130,5,100,500");
  xQueueSend(Movement_Queue, &cmd, portMAX_DELAY);
  delay(6000);

  sprintf(cmd, "V3,8,130,90,5,100,500");
  xQueueSend(Movement_Queue, &cmd, portMAX_DELAY);
  delay(6000);
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

messageParts processQueueMessage(const std::string msg, const std::string from)
{
  //std::vector<std::string> strings;
  std::istringstream f(msg);
  std::string part;

  messageParts mParts = {};
  strcpy(mParts.fullMessage, msg.c_str());
  int index = 0;

  // messageParts2 mParts2 = {};
  // mParts2.fullMessage = msg;

  // while (std::getline(f, part, ','))
  // {
  //   mParts2.values.push_back(part);
  // }

  // mParts2.identifier = mParts2.values.front();
  // mParts2.values.erase(mParts2.values.begin());

  //Serial.println(msg.c_str());

  while (std::getline(f, part, ','))
  {
    //strings.push_back(s);

    //Serial.println(part.c_str());

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
