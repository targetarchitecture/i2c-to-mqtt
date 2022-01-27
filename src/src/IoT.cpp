#include <Arduino.h>
#include "IoT.h"

WiFiClient client;
PubSubClient MQTTClient;

std::string mqtt_server;   //= "192.168.1.189";
std::string mqtt_user;     // = "public";
std::string mqtt_password; // = "public";

QueueHandle_t MQTT_Publish_Queue;

struct MessageToPublish
{
  char topic[100];
  char payload[100];
};

TaskHandle_t MQTTCommandTask;
TaskHandle_t MQTTPublishTask;

volatile bool ConnectSubscriptions = true;

std::vector<std::string> SubscribedTopics;
std::vector<std::string> UnsubscribedTopics;

void MQTT_setup()
{
  MQTT_Publish_Queue = xQueueCreate(10, sizeof(MessageToPublish));

  //read from NVM
  //preferences.putString("mqtt_server", "robotmqtt");
  mqtt_server = preferences.getString("mqtt_server", "").c_str();
  mqtt_user = preferences.getString("mqtt_user", "public").c_str();
  mqtt_password = preferences.getString("mqtt_password", "public").c_str();

  Serial << "MQTT Server from NVM:" << mqtt_server.c_str() << endl;

  //if value not set then just bail out
  if (mqtt_server == "")
  {
    return;
  }

  //set this up as early as possible
  MQTTClient.setClient(client);
  MQTTClient.setServer(mqtt_server.c_str(), 1883);
  MQTTClient.setCallback(recieveMessage);

  xTaskCreatePinnedToCore(
      MQTT_command_task,   /* Task function. */
      "MQTT Command Task", /* name of task. */
      2046,                /* Stack size of task (uxTaskGetStackHighWaterMark:1336) */
      NULL,                /* parameter of the task */
      MQTT_task_Priority,  /* priority of the task */
      &MQTTCommandTask,    /* Task handle to keep track of created task */
      1);

  xTaskCreatePinnedToCore(
      MQTT_Publish_task,         /* Task function. */
      "MQTT Publish Task",       /* name of task. */
      5000,                      /* Stack size of task (uxTaskGetStackHighWaterMark:16084) */
      NULL,                      /* parameter of the task */
      MQTT_client_task_Priority, /* priority of the task */
      &MQTTPublishTask,          /* Task handle to keep track of created task */
      1);
}

void recieveMessage(char *topic, byte *payload, unsigned int length)
{
  //Serial << "Message arrived [" << topic << "]" << endl;

  std::string receivedMsg;

  for (int i = 0; i < length; i++)
  {
    char c = payload[i];

    //Serial.print(c);

    receivedMsg += c;
  }
  //Serial.println();

  char msgtosend[MAXBBCMESSAGELENGTH];
  sprintf(msgtosend, "MQTT:'%s','%s'", topic, receivedMsg.c_str());
  sendToMicrobit(msgtosend);
}

void checkMQTTconnection()
{
  Serial.println("MQTTClient NOT Connected :(");
  u_long startTime = millis();

  do
  {
    if (MQTTClient.connected() == true)
    {
      break;
    }

    //get the unique id into a variable
    std::ostringstream getEfuseMac;
    getEfuseMac << BOARDNAME << "_" << ESP.getEfuseMac();
    std::string mqtt_client = getEfuseMac.str();

    Serial << mqtt_client.c_str() << ":" << mqtt_user.c_str() << ":" << mqtt_password.c_str() << endl;

    MQTTClient.connect(mqtt_client.c_str(), mqtt_user.c_str(), mqtt_password.c_str());

    delay(500);

  } while (1);

  Serial << "MQTTClient now connected in " << millis() - startTime << "ms :)" << endl;

  //Serial.println("MQTTClient now connected :)");

  //set to true to get the subscriptions setup again
  ConnectSubscriptions = true;
}

void setupSubscriptions()
{
  // Serial.print("setupSubscriptions");

  // Serial << "SubscribedTopics =" << SubscribedTopics.size() << endl;

  for (int i = 0; i < SubscribedTopics.size(); i++)
  {
    Serial << "subscribing to:" << SubscribedTopics[i].c_str() << endl;

    MQTTClient.subscribe(SubscribedTopics[i].c_str());
  }

  for (int i = 0; i < UnsubscribedTopics.size(); i++)
  {
    Serial << "unsubscribing to:" << UnsubscribedTopics[i].c_str() << endl;

    MQTTClient.unsubscribe(UnsubscribedTopics[i].c_str());
  }
}

void MQTT_Publish_task(void *pvParameter)
{
  // UBaseType_t uxHighWaterMark;
  // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  // Serial.print("MQTT_Publish_task uxTaskGetStackHighWaterMark:");
  // Serial.println(uxHighWaterMark);

  for (;;)
  {
    if (WiFi.isConnected() == false)
    {
      //  Serial.println("WiFi.isConnected() == false");

      delay(500);
    }
    else
    {
      if (MQTTClient.connected() == false)
      {
        // Serial.println("checkMQTTconnection");

        checkMQTTconnection();
      }
      else
      {
        //check to see if we need to remake the subscriptions
        if (ConnectSubscriptions == true)
        {
          //   Serial.println("ConnectSubscriptions == true");

          //set up subscription topics
          setupSubscriptions();

          ConnectSubscriptions = false;
        }

        MessageToPublish msg;

        //check the message queue and if empty just proceed passed
        if (xQueueReceive(MQTT_Publish_Queue, &msg, 0) == pdTRUE)
        {
          // Serial.print("publish topic:");
          // Serial.print(msg.topic);
          // Serial.print("\t");
          // Serial.print("payload:");
          // Serial.print(msg.payload);
          // Serial.println("");

          MQTTClient.publish(msg.topic, msg.payload);
        }

        //Serial.println("MQTTClient.loop()");

        //must always call the loop method - if we have wifi and a connection
        MQTTClient.loop();
      }

      delay(100);
    }
  }
  vTaskDelete(NULL);
}

void MQTT_command_task(void *pvParameter)
{
  // UBaseType_t uxHighWaterMark;
  // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  // Serial.print("MQTT_command_task uxTaskGetStackHighWaterMark:");
  // Serial.println(uxHighWaterMark);

  for (;;)
  {
    messageParts parts;

    //wait for new MQTT command in the queue
    xQueueReceive(MQTT_Command_Queue, &parts, portMAX_DELAY);

    //Serial.print("MQTT_Command_Queue:");
    //Serial.println(parts.identifier);

    std::string identifier = parts.identifier;

    if (identifier.compare("PUBLISH") == 0)
    {
      struct MessageToPublish msg;
      std::istringstream f(parts.part1);
      std::string part;
      int index = 0;

      while (std::getline(f, part, '|'))
      {
        if (index == 0)
        {
          strcpy(msg.topic, part.c_str());
        }
        if (index == 1)
        {
          strcpy(msg.payload, part.c_str());
        }
        index++;
      }

      // Serial.print("queue message:");
      // Serial.print(msg.topic);
      // Serial.print("\t");
      // Serial.print("payload:");
      // Serial.println(msg.payload);

      xQueueSend(MQTT_Publish_Queue, &msg, portMAX_DELAY);
    }
    else if (identifier.compare("SUBSCRIBE") == 0)
    {
      std::string topic = parts.part1;
      //  strcpy(topic, parts.part1);

      //Serial << "Subscribe topic:" << topic.c_str() << endl;

      subscribe(topic);
    }
    else if (identifier.compare("UNSUBSCRIBE") == 0)
    {
      std::string topic = parts.part1;

      unsubscribe(topic);
    }
  }

  vTaskDelete(NULL);
}

void unsubscribe(std::string topic)
{
  //Serial << "UnsubscribedTopics =" << SubscribedTopics.size() << endl;

  // char t[100];
  // strcpy(t, topic);

  UnsubscribedTopics.push_back(topic);

  //Serial << "UnsubscribedTopics =" << SubscribedTopics.size() << endl;

  //set to true to get the subscriptions setup again
  ConnectSubscriptions = true;
}

void subscribe(std::string topic)
{
  //Serial << "Subscribe cmd:" << topic.c_str() << endl;

  // char t[100];

  // strcpy(t, topic);

  SubscribedTopics.push_back(topic);

  // Serial << "SubscribedTopics =" << SubscribedTopics.size() << endl;

  //set to true to get the subscriptions setup again
  ConnectSubscriptions = true;
}