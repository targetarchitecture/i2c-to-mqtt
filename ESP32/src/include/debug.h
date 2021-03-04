#pragma once

#include "MQTT.h"
//#include <stdarg.h>
#include <vector>
#include <sstream>
#include <iostream>

extern PubSubClient MQTTClient;

// struct MQTTMessage
// {
//   std::string topic;
//   std::string payload;
// };


// template <class... Args>

//int printx(const char* fmt...);

void foo(const char *format...);

// {
//   Serial.printf(format.c_str(), args...);

//   if (MQTTClient.connected() == true)
//   {
//     char payload[MAXBBCMESSAGELENGTH];
//     sprintf(payload, format.c_str(), args...);

//     auto chipID = ESP.getEfuseMac();
//     std::string topic;

//     topic = "debug/";
//     topic += chipID;

//     MQTTMessage msg = {topic, payload};

//     xQueueSend(MQTT_Message_Queue, &msg, portMAX_DELAY);
//   }
// }