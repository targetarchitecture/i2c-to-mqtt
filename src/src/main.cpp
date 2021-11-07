/* 
Rainbow Sparkle Unicorn - SN9
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
#include <Streaming.h>

#include "FS.h"
#include <LITTLEFS.h>
#include <WiFi.h>

#include "messaging.h"
#include "microbit-uart.h"
#include "sound.h"
#include "encoders.h"
#include "touch.h"
#include "DAC.h"
#include "ADC.h"
#include "light.h"
#include "switch.h"
#include "movement.h"
//#include "IoT.h"
#include "WifiMgr.h"

void checkI2Cerrors(std::string area);
void runTests();

QueueHandle_t Sound_Queue;
QueueHandle_t DAC_Queue;
QueueHandle_t Light_Queue;
QueueHandle_t Movement_Queue;
QueueHandle_t MQTT_Queue;

SemaphoreHandle_t i2cSemaphore;

extern std::string requestMessage;

extern void dealWithMessage(std::string message);

std::string RainbowSparkleUnicornName;

void setup()
{
  //stop bluetooth
  btStop();

  //Set UART log level
  esp_log_level_set("SN9", ESP_LOG_VERBOSE);

  pinMode(ONBOARDLED, OUTPUT);

  //start i2c
  Wire.begin(SDA, SCL);

  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println("");
  Serial.println("");

  //create i2c Semaphore , and set to useable
  i2cSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(i2cSemaphore);

  //set up the main queues
  Sound_Queue = xQueueCreate(5, sizeof(messageParts));
  DAC_Queue = xQueueCreate(5, sizeof(messageParts));
  Light_Queue = xQueueCreate(30, sizeof(messageParts));
  Movement_Queue = xQueueCreate(30, sizeof(messageParts));
  MQTT_Queue = xQueueCreate(30, sizeof(messageParts));

  //get the unique id into a variable (once)
  std::ostringstream getEfuseMac;
  getEfuseMac << "SN9_" << ESP.getEfuseMac();
  RainbowSparkleUnicornName = getEfuseMac.str();

  //get wifi going first as this seems to be problematic
  Wifi_setup();

  //call the feature setup methods
  microbit_setup();

  sound_setup();

  touch_setup();

  encoders_setup();

  DAC_setup();

  ADC_setup();

  light_setup();

  switch_setup();

  movement_setup();

  //MQTT_setup(RainbowSparkleUnicornName);

  Serial << "SN9 completed in " << millis() << "ms" << endl;

  runTests();
}

void POST(uint8_t flashes)
{
  //TODO: debate which tasks need stopping?
  vTaskSuspendAll(); //added on 31/1/21

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

void checkI2Cerrors(std::string area)
{
  if (Wire.lastError() != 0)
  {
    //Serial << "i2C error @ " << area.c_str() << ":" << Wire.getErrorText(Wire.lastError()) << endl;

    Wire.clearWriteError();
  }
}

void loop()
{
// digitalWrite(ONBOARDLED, WiFi.isConnected());

  if (WiFi.isConnected() == true)
  {
    digitalWrite(ONBOARDLED, HIGH);
  }
  else
  {
    digitalWrite(ONBOARDLED, LOW);
  }
  
  delay(5000);
}

void runTests()
{
  dealWithMessage("SUBSCRIBE,ps2/buttons ");

  // dealWithMessage("STARTING ");
  // dealWithMessage("MLINEAR,8,0,180,10,100,500 ");

  // dealWithMessage("MPWM,8,500 ");
}
