#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include "messaging.h"
#include <list>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

void MQTT_setup();
void checkMQTTconnection();
void Wifi_connect();

void MQTT_deal_with_message(char msg[UARTMESSAGELENGTH]);
void MQTT_task(void *pvParameter);
void setupSubscriptions();
void recieveMessage(char *topic, byte *payload, unsigned int length);
void MQTTClient_task(void *pvParameter);
void sendMQTTConnectionStatus();

extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern void checkI2Cerrors(const char *area);

extern QueueHandle_t MQTT_Queue;
extern messageParts processQueueMessage(const std::string msg, const std::string from);

extern char TXtoBBCmessage[MAXBBCMESSAGELENGTH];
extern char RXfromBBCmessage[UARTMESSAGELENGTH];