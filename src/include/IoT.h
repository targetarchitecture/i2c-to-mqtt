#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include "messaging.h"
#include <list>
#include "esp_wifi.h"

void MQTT_setup(std::string RainbowSparkleUnicornName);
void checkMQTTconnection();
//void Wifi_connect();

void subscribe(std::string topic);
void unsubscribe(std::string topic);

void MQTT_deal_with_message(char msg[UARTMESSAGELENGTH]);
void MQTT_task(void *pvParameter);
void setupSubscriptions();
void recieveMessage(char *topic, byte *payload, unsigned int length);
void MQTT_Publish_task(void *pvParameter);
//void sendMQTTConnectionStatus();

//extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern void checkI2Cerrors(const char *area);

extern QueueHandle_t MQTT_Queue;
extern messageParts processQueueMessage(const std::string msg, const std::string from);

extern char TXtoBBCmessage[MAXBBCMESSAGELENGTH];
extern char RXfromBBCmessage[UARTMESSAGELENGTH];

//extern void Wifi_setup();

//extern SemaphoreHandle_t wifiSemaphore;

extern std::string RainbowSparkleUnicornName;
