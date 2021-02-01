#include <Arduino.h>
#include "MQTT.h"

WiFiClient client;
PubSubClient MQTTClient;

std::string MQTT_SERVER = "";
std::string MQTT_CLIENTID = "";
std::string MQTT_USERNAME = "";
std::string MQTT_KEY = "";
std::string WIFI_SSID = "";
std::string WIFI_PASSPHRASE = "";

std::string IP_ADDRESS = "";

QueueHandle_t MQTT_Queue;
QueueHandle_t MQTT_Message_Queue;

unsigned long lastMQTTStatusSent = 0;

struct MQTTMessage
{
  std::string topic;
  std::string payload;
};

struct MQTTSubscription
{
  std::string topic;
  bool subscribe;
};

TaskHandle_t MQTTTask;
TaskHandle_t MQTTClientTask;

volatile bool ConnectWifi = false;
volatile bool ConnectMQTT = false;
volatile bool ConnectSubscriptions = true;

std::list<MQTTSubscription> MQTTSubscriptions;

void MQTT_setup()
{
  pinMode(ONBOARDLED, OUTPUT);

  MQTT_Queue = xQueueCreate(50, sizeof(RXfromBBCmessage));
  MQTT_Message_Queue = xQueueCreate(50, sizeof(struct MQTTMessage));

  xTaskCreatePinnedToCore(
      MQTT_task,          /* Task function. */
      "MQTT Task",        /* name of task. */
      17000,              /* Stack size of task (uxTaskGetStackHighWaterMark:16084) */
      NULL,               /* parameter of the task */
      MQTT_task_Priority, /* priority of the task */
      &MQTTTask,          /* Task handle to keep track of created task */
      1);

  xTaskCreatePinnedToCore(
      MQTTClient_task,           /* Task function. */
      "MQTTClient Task",         /* name of task. */
      17000,                     /* Stack size of task (uxTaskGetStackHighWaterMark:16084) */
      NULL,                      /* parameter of the task */
      MQTT_client_task_Priority, /* priority of the task */
      &MQTTClientTask,           /* Task handle to keep track of created task */
      1);
}

void Wifi_connect()
{
  // Serial.println("Connecting to Wifi");
  // Serial.println(WIFI_SSID.c_str());
  // Serial.println(WIFI_PASSPHRASE.c_str());

  WiFi.mode(WIFI_OFF);
  delay(250);
  WiFi.mode(WIFI_STA);
  delay(250);

  //connect
  uint32_t speed = 500;
  while (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSPHRASE.c_str());

    digitalWrite(ONBOARDLED, HIGH);
    delay(speed);
    digitalWrite(ONBOARDLED, LOW);
    delay(speed);

    //Serial.println(WiFi.status());
  }

  char msgtosend[MAXBBCMESSAGELENGTH];
  sprintf(msgtosend, "G1,%s", WiFi.localIP().toString().c_str());

  sendToMicrobit(msgtosend);
}

// void MQTT_connect2()
// {
//   MQTTClient.setClient(client);
//   MQTTClient.setServer(MQTT_SERVER.c_str(), 1883);
//   MQTTClient.setCallback(recieveMessage);

//   checkMQTTconnection(true);

// //connect
// uint32_t speed = 250;
// for (size_t i = 0; i < 4; i++)
// {
//   digitalWrite(ONBOARDLED, HIGH);
//   delay(speed);
//   digitalWrite(ONBOARDLED, LOW);
//   delay(speed);
// }
//}

void recieveMessage(char *topic, byte *payload, unsigned int length)
{
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");

  std::string receivedMsg;

  for (int i = 0; i < length; i++)
  {
    char c = payload[i];

    //Serial.print(c);

    receivedMsg += c;
  }
  //Serial.println();

  char msgtosend[MAXBBCMESSAGELENGTH];
  sprintf(msgtosend, "G3,'%s','%s'", topic, receivedMsg.c_str());
  sendToMicrobit(msgtosend);
}

void checkMQTTconnection()
{
  if (!MQTTClient.connected())
  {
    //Serial.println("!MQTTClient.connected()");

    delay(10);

    bool MQTTConnected = MQTTClient.connect(MQTT_CLIENTID.c_str(), MQTT_USERNAME.c_str(), MQTT_KEY.c_str());

    if (MQTTConnected == true)
    {
      //set to true to get the subscriptions setup again
      ConnectSubscriptions = true;
    }

    unsigned long currentMillis = millis();

    if (currentMillis - lastMQTTStatusSent >= 1000)
    {
      lastMQTTStatusSent = currentMillis;

      if (MQTTConnected == true)
      {
        //Serial.println("G2,1");

        char msgtosend[MAXBBCMESSAGELENGTH];
        sprintf(msgtosend, "G2,1");
        sendToMicrobit(msgtosend);

        //set to true to get the subscriptions setup again
        ConnectSubscriptions = true;
      }
      else
      {
        //Serial.println("G2,0");

        char msgtosend[MAXBBCMESSAGELENGTH];
        sprintf(msgtosend, "G2,0");
        sendToMicrobit(msgtosend);
      }
    }
  }
}

void setupSubscriptions()
{
  std::list<MQTTSubscription>::iterator it;

  for (it = MQTTSubscriptions.begin(); it != MQTTSubscriptions.end(); it++)
  {
    // Access the object through iterator
    //auto topic = it->topic.c_str();
    auto subscribe = it->subscribe;

    if (subscribe == true)
    {
      //Serial.print("subscribing to:");
      //Serial.println(it->topic.c_str());

      MQTTClient.subscribe(it->topic.c_str());
    }
    else
    {
      //Serial.print("unsubscribing to:");
      //Serial.println(it->topic.c_str());

      MQTTClient.unsubscribe(it->topic.c_str());
    }
  }
}

void MQTTClient_task(void *pvParameter)
{
  MQTTMessage msg;

  for (;;)
  {
    if (ConnectWifi == true)
    {
      //Serial.println("ConnectWifi == true");

      if (WiFi.isConnected() == false)
      {
        //Serial.println("WiFi.isConnected() == false");

        Wifi_connect();
      }

      if (ConnectMQTT == true)
      {
        //Serial.println("ConnectMQTT == true");

        //only bother if actually connected to internet!!
        if (WiFi.isConnected() == true)
        {
          //Serial.println("WiFi.isConnected() == true");

          checkMQTTconnection();

          if (MQTTClient.connected())
          {
            //Serial.println("MQTTClient.connected()");

            //check the message queue and if empty just proceed passed
            if (xQueueReceive(MQTT_Message_Queue, &msg, 0) == pdTRUE)
            {
              // Serial.print("publish topic:");
              // Serial.print(msg.topic.c_str());
              // Serial.print("\t\t");
              // Serial.print("payload:");
              // Serial.print(msg.payload.c_str());
              // Serial.println("");

              MQTTClient.publish(msg.topic.c_str(), msg.payload.c_str());
            }

            //check to see if we need to remake the subscriptions
            if (ConnectSubscriptions == true)
            {
              //Serial.println("ConnectSubscriptions == true");

              //set up subscription topics
              setupSubscriptions();

              ConnectSubscriptions = false;
            }

            //must call the loop method!
            MQTTClient.loop();

            //https://github.com/256dpi/arduino-mqtt/blob/master/examples/ESP32DevelopmentBoard/ESP32DevelopmentBoard.ino
            delay(10); // <- fixes some issues with WiFi stability ???
          }
        }
        else
        {
          Serial.println("WiFi not connected,cannot send MQTT message");
        }
      }
    }

    delay(100);
  }

  vTaskDelete(NULL);
}

void MQTT_task(void *pvParameter)
{
  messageParts parts;

  // UBaseType_t uxHighWaterMark;
  // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  // Serial.print("MQTT_task uxTaskGetStackHighWaterMark:");
  // Serial.println(uxHighWaterMark);

  for (;;)
  {
    char msg[MAXESP32MESSAGELENGTH] = {0};

    //show WiFi connection
    if (WiFi.isConnected() == true)
    {
      digitalWrite(ONBOARDLED, HIGH);
    }
    else
    {
      digitalWrite(ONBOARDLED, LOW);
    }

    //wait for new MQTT command in the queue
    xQueueReceive(MQTT_Queue, &msg, portMAX_DELAY);

    // Serial.print("MQTT_Queue:");
    // Serial.println(msg);

    //TODO: see if need this copy of msg
    std::string X = msg;

    parts = processQueueMessage(X.c_str(), "MQTT");

    if (strcmp(parts.identifier, "T1") == 0)
    {
      std::string str(parts.value1);
      WIFI_SSID = str;
    }
    else if (strcmp(parts.identifier, "T2") == 0)
    {
      std::string str(parts.value1);
      WIFI_PASSPHRASE = str;
    }
    else if (strcmp(parts.identifier, "T3") == 0)
    {
      std::string strA(parts.value1);
      WIFI_SSID = strA;

      std::string strB(parts.value2);
      WIFI_PASSPHRASE = strB;

      ConnectWifi = true;
    }
    else if (strcmp(parts.identifier, "T4") == 0)
    {
      std::string str(parts.value1);
      MQTT_SERVER = str;
    }
    else if (strcmp(parts.identifier, "T5") == 0)
    {
      std::string str(parts.value1);
      MQTT_CLIENTID = str;
    }
    else if (strcmp(parts.identifier, "T6") == 0)
    {
      std::string str(parts.value1);
      MQTT_USERNAME = str;
    }
    else if (strcmp(parts.identifier, "T7") == 0)
    {
      std::string str(parts.value1);
      MQTT_KEY = str;
    }
    else if (strcmp(parts.identifier, "T8") == 0)
    {
      //set server variables
      MQTTClient.setClient(client);
      MQTTClient.setServer(MQTT_SERVER.c_str(), 1883);
      MQTTClient.setCallback(recieveMessage);

      ConnectMQTT = true;

      //set to true to get the subscriptions setup again
      ConnectSubscriptions = true;
    }
    else if (strcmp(parts.identifier, "T9") == 0)
    {
      std::string topic(parts.value1);
      std::string payload(parts.value2);

      MQTTMessage msg = {topic, payload};

      xQueueSend(MQTT_Message_Queue, &msg, portMAX_DELAY);
    }
    else if (strcmp(parts.identifier, "T10") == 0)
    {
      //add to the list (if it doesn't exist)
      std::string topic(parts.value1);
      bool found = false;

      std::list<MQTTSubscription>::iterator it;

      for (it = MQTTSubscriptions.begin(); it != MQTTSubscriptions.end(); it++)
      {
        // Access the object through iterator
        auto MQTTSubscriptionTopic = it->topic;

        if (topic == MQTTSubscriptionTopic)
        {
          //Serial.print("Update to subscription list: ");
          //Serial.println(parts.value1);

          it->subscribe = true;
          found = true;
        }
      }

      if (found == false)
      {
        MQTTSubscriptions.push_back({topic, true});

        //Serial.print("Add to subscription list: ");
        //Serial.println(parts.value1);
      }

      //set to true to get the subscriptions setup again
      ConnectSubscriptions = true;
    }
    else if (strcmp(parts.identifier, "T11") == 0)
    {
      std::string topic(parts.value1);

      std::list<MQTTSubscription>::iterator it;

      for (it = MQTTSubscriptions.begin(); it != MQTTSubscriptions.end(); it++)
      {
        // Access the object through iterator
        auto MQTTSubscriptionTopic = it->topic;

        if (topic == MQTTSubscriptionTopic)
        {
          // Serial.print("Update to unsubsripton list: ");
          // Serial.println(parts.value1);

          it->subscribe = false;
        }
      }

      //set to true to get the subscriptions setup again
      ConnectSubscriptions = true;
    }
    else if (strcmp(parts.identifier, "T12") == 0)
    {
      //turn off WiFi
      WiFi.mode(WIFI_OFF);
      delay(250);

      //turn off LED
      digitalWrite(ONBOARDLED, LOW);
    }
  }

  vTaskDelete(NULL);
}
