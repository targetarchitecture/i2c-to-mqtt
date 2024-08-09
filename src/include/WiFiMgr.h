#pragma once

#include <WiFi.h>
#include <Preferences.h>

void Wifi_setup();
void WiFiEvent(WiFiEvent_t event);

extern Preferences preferences;

extern String storedSSID;
extern String storedWifiPassword;