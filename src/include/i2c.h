#pragma once

#include <Wire.h>
#include <map>
#include <Preferences.h>
#include <PubSubClient.h>

void ReceivedCommand(int);
void SendData();
void i2c_setup();

extern Preferences preferences;

extern std::string mqtt_server;   //= "192.168.1.189";
extern std::string mqtt_user;     // = "public";
extern std::string mqtt_password; // = "public";

extern std::map<std::string, std::string> mqtt_topics;

// extern std::string mqtt_topic;
// extern std::string mqtt_topic_value;

extern std::string storedSSID;
extern std::string storedWifiPassword;

extern PubSubClient mqttClient;