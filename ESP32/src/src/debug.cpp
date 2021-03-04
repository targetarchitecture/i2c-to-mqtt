#include <Arduino.h>
#include "debug.h"

//void foo(const std::string *format...);

void foo(const char *format...)
{
  va_list args;

  va_start(args, format);
  // vfprintf(logfp, fmt, args);

  char payload[MAXBBCMESSAGELENGTH];
  snprintf(payload, MAXBBCMESSAGELENGTH, format, args);

  Serial.println(payload);

  va_end(args);

  if (MQTTClient.connected() == true)
  {
    //  char payload[MAXBBCMESSAGELENGTH];
    //  snprintf(payload,MAXBBCMESSAGELENGTH, format, args);

    auto chipID = ESP.getEfuseMac();
    std::string topic;

    topic = "debug";
    //topic += chipID;

    MQTTMessage msg = {topic, payload};

    xQueueSend(MQTT_Message_Queue, &msg, portMAX_DELAY);
  }
}

// template <class... Args>
// void foo(const std::string &format, Args... args)
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