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

TaskHandle_t MQTTTask;

void MQTT_setup()
{
  pinMode(ONBOARDLED, OUTPUT);

  xTaskCreatePinnedToCore(
      MQTT_task,     /* Task function. */
      "MQTT Task",   /* name of task. */
      17000,         /* Stack size of task (uxTaskGetStackHighWaterMark:16084) */
      NULL,          /* parameter of the task */
      5,             /* priority of the task */
      &MQTTTask, 1); /* Task handle to keep track of created task */
}

void Wifi_connect()
{
  // Serial.println("Connecting to Wifi");
  // Serial.println(WIFI_SSID.c_str());
  // Serial.println(WIFI_PASSPHRASE.c_str());

  WiFi.mode(WIFI_OFF);
  delay(250);
  WiFi.mode(WIFI_STA);

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

void checkMQTTconnection(const bool signalToMicrobit)
{
  if (!MQTTClient.connected())
  {
    if (MQTTClient.connect(MQTT_CLIENTID.c_str(), MQTT_USERNAME.c_str(), MQTT_KEY.c_str()))
    {
      char msgtosend[MAXBBCMESSAGELENGTH];
      sprintf(msgtosend, "G2,1");
      sendToMicrobit(msgtosend);
    }
    else
    {
      char msgtosend[MAXBBCMESSAGELENGTH];
      sprintf(msgtosend, "G2,0");
      sendToMicrobit(msgtosend);
    }
  }
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

    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("MQTT_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    //show WiFi connection
    if (WiFi.isConnected() == true)
    {
      digitalWrite(ONBOARDLED, HIGH);
    }
    else
    {
      digitalWrite(ONBOARDLED, LOW);
    }

    //wait for new music command in the queue
    xQueueReceive(MQTT_Queue, &msg, portMAX_DELAY);

    // Serial.print("MQTT_Queue:");
    // Serial.println(msg);

    //TODO: see if need this copy of msg
    std::string X = msg;

    parts = processQueueMessage(X.c_str(), "MQTT");

    if (strcmp(parts.identifier, "T1") == 0)
    {
      std::string str(parts.value1);

      //TODO: Better way of sending spaces
      WIFI_SSID = ReplaceString(str, "PPP", " ");

      //Serial.printf("\n\nWIFI_SSID:%s<<<\n\n\n", WIFI_SSID.c_str());
    }
    else if (strcmp(parts.identifier, "T2") == 0)
    {
      std::string str(parts.value1);

      WIFI_PASSPHRASE = str;

      //Serial.printf("\n\n_WIFI_PASSPHRASE:%s<<<\n\n\n", WIFI_PASSPHRASE.c_str());
    }
    else if (strcmp(parts.identifier, "T3") == 0)
    {
      Wifi_connect();
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
      //only bother if actually connected to internet!!
      if (WiFi.isConnected() == true)
      {
        MQTT_connect();
      }
    }
    else if (strcmp(parts.identifier, "T9") == 0)
    {
      //only bother if actually connected to internet!!
      if (WiFi.isConnected() == true)
      {
        checkMQTTconnection(false);

        std::string topic(parts.value1);
        std::string payload(parts.value2);

        MQTTClient.publish(topic.c_str(), payload.c_str());
      }
      else
      {
        Serial.println("WiFi not connected,cannot send MQTT message");
      }
    }
  }

  vTaskDelete(NULL);
}

std::string ReplaceString(std::string subject, const std::string &search,
                          const std::string &replace)
{
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos)
  {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
  return subject;
}
