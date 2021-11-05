#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include "WifiMgr.h"
#include <Stream.h>
#include <sstream>

//define your default values here, if there are different values in config.json, they are overwritten.
std::string mqtt_server;
std::string mqtt_user;
std::string mqtt_password;
std::string mqtt_client;

TaskHandle_t WifiTask;

void Wifi_setup()
{
    //Wifi_task(NULL);

    xTaskCreatePinnedToCore(
        Wifi_task,     /* Task function. */
        "Wifi Task",   /* name of task. */
        10000,         /* Stack size of task (uxTaskGetStackHighWaterMark = 3196) */
        NULL,          /* parameter of the task */
        20,            /* priority of the task */
        &WifiTask, 0); /* Task handle to keep track of created task */
}

void Wifi_task(void *pvParameters)
{
    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    auto s = millis();

    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", "robotmqtt", 22);
    WiFiManagerParameter custom_mqtt_username("username", "user name", "public", 12);
    WiFiManagerParameter custom_mqtt_password("password", "password", "public", 12);
    WiFiManagerParameter custom_mqtt_client("client", "mqtt client", "SN9", 23);

    //add all your parameters here
    wm.addParameter(&custom_mqtt_server);
    wm.addParameter(&custom_mqtt_username);
    wm.addParameter(&custom_mqtt_password);
    wm.addParameter(&custom_mqtt_client);

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    //wm.resetSettings();

    // The chip ID is essentially its MAC address(length: 6 bytes).
    // uint64_t chipid = ESP.getEfuseMac();
    // char *tmp;
    // sprintf(tmp, "SN9_%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);

    // std::ostringstream stringStream;
    // stringStream << "SN9_" << ESP.getEfuseMac();
    // mqtt_client = stringStream.str();

    bool res;
    res = wm.autoConnect("rsu", "password"); // password protected ap

    //Rainbow Sparkle Unicorn

    if (!res)
    {
        Serial.println(F("Failed to connect"));
        // ESP.restart();
    }
    else
    {
        //if you get here you have connected to the WiFi
        Serial.print(F("connected in "));
        Serial.print(millis() - s);
        Serial.println(F("ms"));
    }

    //read updated parameters
    mqtt_server = custom_mqtt_server.getValue();
    mqtt_user = custom_mqtt_username.getValue();
    mqtt_password = custom_mqtt_password.getValue();
    mqtt_client = custom_mqtt_server.getValue();

    //now run MQTT setup
    //MQTT_setup();

    delay(10 * 1000);

    vTaskDelete(NULL);
}