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
#include <Streaming.h>

#include "messageParts.h"
#include "microbit-i2c.h"
#include "sound.h"
#include "encoders.h"
#include "touch.h"
#include "DAC.h"
#include "ADC.h"
#include "light.h"
#include "switch.h"
#include "movement.h"

void checkI2Cerrors(std::string area);
void runTests();

QueueHandle_t Sound_Queue;
QueueHandle_t DAC_Queue;
QueueHandle_t Light_Queue;
QueueHandle_t Movement_Queue;

extern std::string requestMessage;
extern SemaphoreHandle_t i2cSemaphore;

void setup()
{
  //Set UART log level
  esp_log_level_set("SN7", ESP_LOG_VERBOSE);

  //stop bluetooth
  btStop();

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

  //MQTT_Queue = xQueueCreate(50, sizeof(MAXESP32MESSAGELENGTH));
  //MQTT_Message_Queue = xQueueCreate(50, sizeof(struct MQTTMessage));

  //get wifi going first as this seems to be problematic
  //MQTT_setup();

  //call the feature setup methods
  sound_setup();

  touch_setup();

  encoders_setup();

  DAC_setup();

  ADC_setup();

  light_setup();

  switch_setup();

  movement_setup();

  microbit_i2c_setup();

  Serial << "SN7 completed in " << millis() << "ms" << endl;

  //runTests();
}


void POST(uint8_t flashes)
{
  //TODO: debate which tasks need stopping?
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

void checkI2Cerrors(std::string area)
{
  if (Wire.lastError() != 0)
  {
    Serial << "i2C error @ " << area.c_str() << ":" << Wire.getErrorText(Wire.lastError()) << endl;

    Wire.clearWriteError();
  }
}

void loop()
{
  delay(1000);
}

void runTests()
{
  dealWithMessage("STARTING");

  dealWithMessage("DIAL1,128");
  dealWithMessage("DIAL2,254");

  dealWithMessage("MANGLE,15,45,100,505");
  dealWithMessage("LLEDALLON");

  dealWithMessage("TUPDATE");

  Serial << "Touched: " << requestMessage.c_str() << endl;

  dealWithMessage("SUPDATE");

  Serial << "Switched: " << requestMessage.c_str() << endl;

  dealWithMessage("ROTARY1");

  Serial << "ROTARY1: " << requestMessage.c_str() << endl;

  dealWithMessage("ROTARY2");

  Serial << "ROTARY2: " << requestMessage.c_str() << endl;

  dealWithMessage("SLIDER1");

  Serial << "SLIDER1: " << requestMessage.c_str() << endl;

  dealWithMessage("SLIDER2");

  Serial << "SLIDER2: " << requestMessage.c_str() << endl;

  delay(1000);

  dealWithMessage("SBUSY");

  Serial << "SBUSY: " << requestMessage.c_str() << endl;

  delay(1000);

  dealWithMessage("SPLAY,1");
  delay(1000);

  dealWithMessage("SBUSY");

  Serial << "SBUSY: " << requestMessage.c_str() << endl;

  delay(1000);
  dealWithMessage("SPAUSE");
  delay(1000);
  dealWithMessage("SRESUME");
  delay(1000);
  dealWithMessage("SSTOP");

  dealWithMessage("SBUSY");

  Serial << "SBUSY: " << requestMessage.c_str() << endl;

  delay(5000);
  dealWithMessage("SPLAY,3");
}
