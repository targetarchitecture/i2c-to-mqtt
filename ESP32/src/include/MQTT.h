#ifndef MQTT_h
#define MQTT_h

#include <WiFi.h>
#include <PubSubClient.h>
#include "messageParts.h"
#include <list>

void MQTT_setup();
void MQTT_connect();
void checkMQTTconnection(const bool signalToMicrobit);
void Wifi_connect();

void MQTT_deal_with_message(char msg[MAXESP32MESSAGELENGTH]);
void MQTT_task(void *pvParameter);
void setupSubscriptions();
void recieveMessage(char *topic, byte *payload, unsigned int length);

extern void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH]);
extern void checkI2Cerrors(const char *area);

extern QueueHandle_t MQTT_Queue;
extern messageParts processQueueMessage(const std::string msg, const std::string from);

#endif
