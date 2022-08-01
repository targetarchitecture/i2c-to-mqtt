#pragma once

#include <Stream.h>
#include <sstream>
#include <WiFi.h>
#include <Preferences.h>
#include <Streaming.h>

void Wifi_setup();
void WiFiEvent(WiFiEvent_t event);

extern Preferences preferences;