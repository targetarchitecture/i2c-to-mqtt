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

QueueHandle_t MQTT_Message_Queue;

struct MQTTMessage
{
  std::string topic;
  std::string payload;
};

TaskHandle_t MQTTTask;
TaskHandle_t MQTTClientTask;

bool ConnectWifi = false;
bool ConnectMQTT = false;

struct MQTTSubscription
{
  std::string topic;
  bool subscribe;
};

std::list<MQTTSubscription> MQTTSubscriptions;

void MQTT_setup()
{
  pinMode(ONBOARDLED, OUTPUT);

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
      MQTTClient_task,    /* Task function. */
      "MQTTClient Task",  /* name of task. */
      17000,              /* Stack size of task (uxTaskGetStackHighWaterMark:16084) */
      NULL,               /* parameter of the task */
      MQTT_task_Priority, /* priority of the task */
      &MQTTClientTask,    /* Task handle to keep track of created task */
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

void MQTT_connect()
{
  MQTTClient.setClient(client);
  MQTTClient.setServer(MQTT_SERVER.c_str(), 1883);
  MQTTClient.setCallback(recieveMessage);

  checkMQTTconnection(true);

  //connect
  uint32_t speed = 250;
  for (size_t i = 0; i < 4; i++)
  {
    digitalWrite(ONBOARDLED, HIGH);
    delay(speed);
    digitalWrite(ONBOARDLED, LOW);
    delay(speed);
  }
}

void recieveMessage(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void checkMQTTconnection(const bool signalToMicrobit)
{
  if (!MQTTClient.connected())
  {
    //Do I need this?
    // yield();

    if (MQTTClient.connect(MQTT_CLIENTID.c_str(), MQTT_USERNAME.c_str(), MQTT_KEY.c_str()))
    {
      char msgtosend[MAXBBCMESSAGELENGTH];
      sprintf(msgtosend, "G2,1");
      sendToMicrobit(msgtosend);

      //set up subscription topics
      setupSubscriptions();

      MQTTClient.subscribe("TEST");
      MQTTClient.publish("TEST", "");
    }
    else
    {
      char msgtosend[MAXBBCMESSAGELENGTH];
      sprintf(msgtosend, "G2,0");
      sendToMicrobit(msgtosend);
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
      Serial.print("subscribing to:");
      Serial.println(it->topic.c_str());

      MQTTClient.subscribe(it->topic.c_str());
    }
    else
    {
      Serial.print("unsubscribing to:");
      Serial.println(it->topic.c_str());

      MQTTClient.unsubscribe(it->topic.c_str());
    }
  }
}

void MQTTClient_task(void *pvParameter)
{
  for (;;)
  {
    if (ConnectWifi == true)
    {
      if (WiFi.isConnected() == false)
      {
        Wifi_connect();
      }

      if (ConnectMQTT == true)
      {
        //only bother if actually connected to internet!!
        if (WiFi.isConnected() == true)
        {
          checkMQTTconnection(false);

          if (MQTTClient.connected())
          {

            xQueueReceive(MQTT_Queue, &msg, portMAX_DELAY);

            MQTTClient.publish();

            //must call the loop method!
            MQTTClient.loop();
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
      ConnectMQTT = true;
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
          Serial.print("Update to subscription list: ");
          Serial.println(parts.value1);

          it->subscribe = true;
          found = true;
        }
      }

      if (found == false)
      {
        MQTTSubscriptions.push_back({topic, true});

        Serial.print("Add to subscription list: ");
        Serial.println(parts.value1);
      }
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
          Serial.print("Update to unsubsripton list: ");
          Serial.println(parts.value1);

          it->subscribe = false;
        }
      }
    }
  }

  vTaskDelete(NULL);
}
