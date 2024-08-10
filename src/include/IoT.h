#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_wifi.h"
#include <Preferences.h>
#include <map>

void MQTT_setup();
void checkMQTTconnection();
void mqttMessageReceived(char *topic, byte *payload, unsigned int length);

extern Preferences preferences;

extern std::string mqtt_server;
extern std::string mqtt_user;
extern std::string mqtt_password;

extern std::map<std::string, std::string> mqtt_topics;

// extern std::string mqtt_topic;
// extern std::string mqtt_topic_value;

