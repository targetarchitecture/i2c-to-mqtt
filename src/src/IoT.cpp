#include <Arduino.h>
#include "IoT.h"
#include "defines.h"

WiFiClient client;
PubSubClient mqttClient;

void MQTT_setup()
{
  // read from NVM
  preferences.begin(BOARDNAME, false);
  mqtt_server = preferences.getString("mqtt_server", "").c_str();
  mqtt_user = preferences.getString("mqtt_user", "public").c_str();
  mqtt_password = preferences.getString("mqtt_password", "public").c_str();
  // mqtt_topic = preferences.getString("mqtt_topic", "").c_str();
  preferences.end();

  Serial.print("MQTT Server from NVM:");
  Serial.println(mqtt_server.c_str());

  // if value not set then just bail out
  if (mqtt_server == "")
  {
    Serial.println("MQTT Server not set");

    return;
  }

  // set this up as early as possible
  mqttClient.setClient(client);
  mqttClient.setServer(mqtt_server.c_str(), 1883);
  mqttClient.setCallback(mqttMessageReceived);
}

void mqttMessageReceived(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("]");

  std::string receivedMsg;

  for (int i = 0; i < length; i++)
  {
    char c = payload[i];

    receivedMsg += c;
  }

  Serial.print("Message:");
  Serial.println(receivedMsg.c_str());

  mqtt_topics[topic] = receivedMsg.c_str();
}

void checkMQTTconnection()
{
  Serial.println("MQTTClient NOT Connected :(");
  u_long startTime = millis();

  do
  {
    if (mqttClient.connected() == true)
    {
      break;
    }

    // get the unique id into a variable
    String wifiMacString = WiFi.macAddress();
    std::string mqtt_client = BOARDNAME;
    mqtt_client = mqtt_client + "_" + wifiMacString.c_str();

    Serial.print(mqtt_client.c_str());
    Serial.print(":");
    Serial.print(mqtt_user.c_str());
    Serial.print(":");
    Serial.println(mqtt_password.c_str());

    mqttClient.connect(mqtt_client.c_str(), mqtt_user.c_str(), mqtt_password.c_str());

    delay(500);

  } while (1);

  Serial.print("MQTTClient now connected in ");
  Serial.print(millis() - startTime);
  Serial.println("ms :)");

  // set to true to get the subscriptions setup again
  for (auto const &x : mqtt_topics)
  {
    mqttClient.subscribe(x.first.c_str());

    Serial.print("MQTTClient now subscribed to ");
    Serial.println(x.first.c_str());
  }
}
