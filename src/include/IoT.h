#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include "messaging.h"
#include "esp_wifi.h"
#include <Streaming.h>
#include <Preferences.h>

void MQTT_setup();
void checkMQTTconnection();

void subscribe(std::string topic);
void unsubscribe(std::string topic);

void MQTT_deal_with_message(char msg[UARTMESSAGELENGTH]);
void MQTT_command_task(void *pvParameter);
void setupSubscriptions();
void recieveMessage(char *topic, byte *payload, unsigned int length);
void MQTT_Publish_task(void *pvParameter);

extern void checkI2Cerrors(const char *area);

extern QueueHandle_t MQTT_Command_Queue;

extern messageParts processQueueMessage(const std::string msg, const std::string from);

extern char TXtoBBCmessage[MAXBBCMESSAGELENGTH];
extern char RXfromBBCmessage[UARTMESSAGELENGTH];

extern std::string RainbowSparkleUnicornName;

extern Preferences preferences;