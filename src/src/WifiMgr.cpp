#include <Arduino.h>
#include <WiFi.h>
#include "WifiMgr.h"
#include <Stream.h>
#include <sstream>

std::string ssid = "152 2.4GHz";
std::string wifi_password = "derwenthorpe";

void Wifi_setup()
{
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid.c_str());

    WiFi.begin(ssid.c_str(), wifi_password.c_str());
}

void Wifi_loop()
{
while (WiFi.isConnected() == false)

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