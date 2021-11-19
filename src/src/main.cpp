/* 
Rainbow Sparkle Unicorn - SN9
*/
#include <Arduino.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <Preferences.h>

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
#include "IoT.h"
#include "WifiMgr.h"

void checkI2Cerrors(std::string area);
void runTests();

Preferences preferences;

QueueHandle_t Sound_Queue;
QueueHandle_t DAC_Queue;
QueueHandle_t Light_Queue;
QueueHandle_t Movement_Queue;
QueueHandle_t MQTT_Command_Queue;

SemaphoreHandle_t i2cSemaphore;

bool POSTerror = false;

extern std::string requestMessage;

extern void dealWithMessage(std::string message);

void setup()
{
  preferences.begin("SN9", false);

  //stop bluetooth
  btStop();

  //Set UART log level
  esp_log_level_set("SN9", ESP_LOG_VERBOSE);

  pinMode(ONBOARDLED, OUTPUT);

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
  Sound_Queue = xQueueCreate(5, sizeof(messageParts));
  DAC_Queue = xQueueCreate(5, sizeof(messageParts));
  Light_Queue = xQueueCreate(30, sizeof(messageParts));
  Movement_Queue = xQueueCreate(30, sizeof(messageParts));
  MQTT_Command_Queue = xQueueCreate(30, sizeof(messageParts));

  //Serial << "Step 1 completed in " << millis() << "ms" << endl;

  //call the feature setup methods
  microbit_setup();

  //  Serial << "Step 2 completed in " << millis() << "ms" << endl;

  //get wifi going first as this seems to be problematic
  Wifi_setup();

  // Serial << "Step 3 completed in " << millis() << "ms" << endl;

  sound_setup();

  // Serial << "Step 4 completed in " << millis() << "ms" << endl;

  touch_setup();

  //  Serial << "Step 5 completed in " << millis() << "ms" << endl;

  encoders_setup();

  //  Serial << "Step 6 completed in " << millis() << "ms" << endl;

  DAC_setup();

  //  Serial << "Step 7 completed in " << millis() << "ms" << endl;

  ADC_setup();

  //  Serial << "Step 8 completed in " << millis() << "ms" << endl;

  light_setup();

  //  Serial << "Step 9 completed in " << millis() << "ms" << endl;

  switch_setup();

  //  Serial << "Step 10 completed in " << millis() << "ms" << endl;

  movement_setup();

  //  Serial << "Step 11 completed in " << millis() << "ms" << endl;

  MQTT_setup();

  Serial << "SN9 completed in " << millis() << "ms" << endl;

  //runTests();
}

void POST(uint8_t flashes)
{
  //TODO: debate which tasks need stopping?
  vTaskSuspendAll(); //added on 31/1/21

  //take back control of LED
  POSTerror = true;

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
  if (Wire.getWriteError() != 0)
  {
    //Serial << "i2C error @ " << area.c_str() << ":" << Wire.getErrorText(Wire.lastError()) << endl;

    Wire.clearWriteError();
  }
}

void loop()
{
  if (POSTerror == false)
  {
    digitalWrite(ONBOARDLED, WiFi.isConnected());
  }

  delay(1000);
}

void runTests()
{
  //dealWithMessage("SUBSCRIBE,ps2/buttons ");

  // dealWithMessage("STARTING ");
  // dealWithMessage("MLINEAR,8,0,180,10,100,500 ");

  // dealWithMessage("MPWM,8,500 ");
}
