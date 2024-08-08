#pragma once

#include <WiFi.h>
#include <PubSubClient.h>
#include "esp_wifi.h"
#include <Preferences.h>

void MQTT_setup();
void checkMQTTconnection();
void mqttMessageReceived(char *topic, byte *payload, unsigned int length);

extern Preferences preferences;