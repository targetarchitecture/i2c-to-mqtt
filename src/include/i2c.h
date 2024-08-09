#pragma once

#include <Wire.h>
#include <Preferences.h>

void ReceivedCommand(int);
void SendData();
void i2c_setup();

extern Preferences preferences;

extern std::string mqtt_server;   //= "192.168.1.189";
extern std::string mqtt_user;     // = "public";
extern std::string mqtt_password; // = "public";
extern std::string mqtt_topic;

extern String storedSSID;
extern String storedWifiPassword;