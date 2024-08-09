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

String storedSSID;
String storedWifiPassword;

std::string mqtt_server;   //= "192.168.1.189";
std::string mqtt_user;     // = "public";
std::string mqtt_password; // = "public";
std::string mqtt_topic;

void setup()
{


  /*
    preferences.putString("ssid", "the robot network");
    preferences.putString("password", "isaacasimov");

    preferences.putString("mqtt_server", "192.168.1.189");
    preferences.putString("mqtt_user", "public");
    preferences.putString("mqtt_password", "public");

    preferences.putUShort("tch_threshold", 12);
    preferences.putUShort("tch_release", 6);
  */

  // Set UART log level
  esp_log_level_set(BOARDNAME, ESP_LOG_VERBOSE);

  pinMode(ONBOARDLED, OUTPUT);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println("");
  Serial.println("");

  // call the microbit first and then the other setup methods
  preferences.begin(BOARDNAME, false);
  Serial.print("NVM ssid:");
  Serial.println(preferences.getString("ssid", ""));
  Serial.print("NVM password:");
  Serial.println(preferences.getString("password", ""));
  Serial.print("NVM mqtt_server:");
  Serial.println(preferences.getString("mqtt_server", ""));
  Serial.print("NVM mqtt_user:");
  Serial.println(preferences.getString("mqtt_user", ""));
  Serial.print("NVM mqtt_password:");
  Serial.println(preferences.getString("mqtt_password", ""));
  Serial.print("NVM mqtt_topic:");
  Serial.println(preferences.getString("mqtt_topic", ""));
  preferences.end();

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
