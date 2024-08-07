#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_wifi.h"
#include <Preferences.h>

void MQTT_setup();
void checkMQTTconnection();

void subscribe(std::string topic);
void unsubscribe(std::string topic);

void MQTT_command_task(void *pvParameter);
void setupSubscriptions();
void recieveMessage(char *topic, byte *payload, unsigned int length);
void MQTT_Publish_task(void *pvParameter);

extern void checkI2Cerrors(const char *area);

extern QueueHandle_t MQTT_Command_Queue;

extern std::string RainbowSparkleUnicornName;

extern Preferences preferences;