/* 
Rainbow Sparkle Unicorn - SN10
*/
#include <Arduino.h>
#include <Wire.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <Preferences.h>
#include <iostream>

#include "defines.h"

#include "IoT.h"
#include "WifiMgr.h"

Preferences preferences;

SemaphoreHandle_t i2cSemaphore;

void setup()
{
  preferences.begin(BOARDNAME, false);

  //Set UART log level
  esp_log_level_set(BOARDNAME, ESP_LOG_VERBOSE);

  //pinMode(ONBOARDLED, OUTPUT);

  //start i2c
  Wire.begin(SDA, SCL);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("");
  Serial.println("");

  //create i2c Semaphore , and set to useable
  i2cSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(i2cSemaphore);

  //call the microbit first and then the other setup methods
  Wifi_setup();
  MQTT_setup();

  Serial << BOARDNAME << " completed in " << millis() << "ms" << std::endl;
}


void loop()
{
  delay(1000);
}

