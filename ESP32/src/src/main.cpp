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

void POST(uint8_t flashes)
{
  //TODO: debate which tasks need stopping?
  //vTaskDelete(ADCTask);
  vTaskSuspendAll(); //added on 31/1/21

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
