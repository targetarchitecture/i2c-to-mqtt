#include <Arduino.h>
#include <WiFi.h>
#include "WifiMgr.h"
#include <Stream.h>
#include <sstream>

std::string ssid = "152 2.4GHz";
std::string wifi_password = "derwenthorpe";

TaskHandle_t WifiTask;

// void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

// void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
// {
//   Serial.println("Disconnected from WiFi access point");
//   Serial.print("WiFi lost connection. Reason: ");
//   Serial.println(info.wifi_sta_disconnected.reason);
// //   Serial.println("Trying to Reconnect");
// //   WiFi.begin(ssid, password);
// }

void Wifi_setup()
{
    xTaskCreatePinnedToCore(
        Wifi_task,     /* Task function. */
        "Wifi Task",   /* name of task. */
        4000,          /* Stack size of task (uxTaskGetStackHighWaterMark = 3196) */
        NULL,          /* parameter of the task */
        1,            /* priority of the task */
        &WifiTask, 0); /* Task handle to keep track of created task */
}

void Wifi_task(void *pvParameters)
{
    auto s = millis();

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid.c_str());

    WiFi.begin(ssid.c_str(), wifi_password.c_str());

    while (WiFi.isConnected() == false)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.print(F("connected in "));
    Serial.print(millis() - s);
    Serial.println(F("ms"));
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    //this task loop will just try to maintain a wifi connection on it's
    for (;;)
    {
        if (WiFi.isConnected() == false)
        {
            Serial.print(millis());
            Serial.println("Reconnecting to WiFi...");
            WiFi.disconnect(true);

            WiFi.begin(ssid.c_str(), wifi_password.c_str());

            while (WiFi.isConnected() == false)
            {
                delay(500);
                Serial.print("#");
            }

            //WiFi.reconnect();

            delay(10 * 1000);
        }
        else
        {
            delay(500);
        }
    }

    vTaskDelete(NULL);
}