#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_wifi.h"
#include <Preferences.h>

void MQTT_setup();
void checkMQTTconnection();
void mqttMessageReceived(char *topic, byte *payload, unsigned int length);

extern Preferences preferences;

extern std::string mqtt_server;   //= "192.168.1.189";
extern std::string mqtt_user;     // = "public";
extern std::string mqtt_password; // = "public";
extern std::string mqtt_topic;

// extern String storedSSID;
// extern String storedWifiPassword;