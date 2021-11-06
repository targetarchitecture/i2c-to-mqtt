#pragma once

void Wifi_setup();
void Wifi_task(void *pvParameters);

extern void MQTT_setup(std::string RainbowSparkleUnicornName);

extern SemaphoreHandle_t wifiSemaphore;