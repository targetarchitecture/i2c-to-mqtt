#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include "WifiMgr.h"


//define your default values here, if there are different values in config.json, they are overwritten.
char mqtt_server[22];
char mqtt_port[6];
char mqtt_user[12];
char mqtt_password[12];
char mqtt_client[23];

TaskHandle_t WifiTask;

void Wifi_setup()
{
    xTaskCreatePinnedToCore(
        Wifi_task,     /* Task function. */
        "Wifi Task",   /* name of task. */
        3500,          /* Stack size of task (uxTaskGetStackHighWaterMark = 3196) */
        NULL,          /* parameter of the task */
        10,            /* priority of the task */
        &WifiTask, 1); /* Task handle to keep track of created task */
}

void Wifi_task(void *pvParameters)
{
    UBaseType_t uxHighWaterMark;
    uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    Serial.print("Wifi_task uxTaskGetStackHighWaterMark:");
    Serial.println(uxHighWaterMark);

    auto s = millis();

    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    //wm.resetSettings();

    // The chip ID is essentially its MAC address(length: 6 bytes).
    uint64_t chipid = ESP.getEfuseMac();
    sprintf(mqtt_client, "SN9_%04X%08X", (uint16_t)(chipid >> 32), (uint32_t)chipid);

    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 22);
    WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
    WiFiManagerParameter custom_mqtt_username("username", "user name", mqtt_user, 12);
    WiFiManagerParameter custom_mqtt_password("password", "password", mqtt_password, 12);
    WiFiManagerParameter custom_mqtt_client("client", "mqtt client", mqtt_client, 23);

    //add all your parameters here
    wm.addParameter(&custom_mqtt_server);
    wm.addParameter(&custom_mqtt_port);
    wm.addParameter(&custom_mqtt_username);
    wm.addParameter(&custom_mqtt_password);
    wm.addParameter(&custom_mqtt_client);

    bool res = wm.autoConnect("Rainbow Sparkle Unicorn", "password"); // password protected ap

    //read updated parameters
    strcpy(mqtt_server, custom_mqtt_server.getValue());
    strcpy(mqtt_port, custom_mqtt_port.getValue());
    strcpy(mqtt_user, custom_mqtt_username.getValue());
    strcpy(mqtt_password, custom_mqtt_password.getValue());
    strcpy(mqtt_client, custom_mqtt_server.getValue());

    if (!res)
    {
        Serial.println("Failed to connect");
        // ESP.restart();
    }
    else
    {
        //if you get here you have connected to the WiFi
        Serial.print("connected in ");
        Serial.print(millis() - s);
        Serial.println("ms");
    }

    vTaskDelete(NULL);
}