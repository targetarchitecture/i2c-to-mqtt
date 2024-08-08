/*
MQTT to i2c
*/
#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include "defines.h"
#include "IoT.h"
#include "WifiMgr.h"
#include "i2c.h"

Preferences preferences;

void setup()
{
  preferences.begin(BOARDNAME, false);

  // Set UART log level
  esp_log_level_set(BOARDNAME, ESP_LOG_VERBOSE);

  pinMode(ONBOARDLED, OUTPUT);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("");
  Serial.println("");

  // call the microbit first and then the other setup methods

  i2c_setup();
  Wifi_setup();
  MQTT_setup();

  Serial.print(BOARDNAME);
  Serial.print(" completed in ");
  Serial.print(millis());
  Serial.println("ms");
}

void loop()
{
  delay(1000);
}
