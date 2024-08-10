#pragma once

#include <WiFi.h>
#include <Preferences.h>

void Wifi_setup();
void WiFiEvent(WiFiEvent_t event);

extern Preferences preferences;

extern std::string storedSSID;
extern std::string storedWifiPassword;