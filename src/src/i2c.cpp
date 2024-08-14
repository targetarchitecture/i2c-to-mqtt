#include <Arduino.h>
#include "i2c.h"
#include "defines.h"

void onRequest()
{
    Serial.println("onRequest");
}

void onReceive(int len)
{
    String command;
    String cmdValue = "";
    String returnValue = "0";

    Serial.printf("ReceivedCommand[%d]: ", len);
    while (Wire.available())
    {
        char c = Wire.read();
        command.concat(c);
    }
    Serial.println(command.c_str());

    int index = command.indexOf(':');

    if (index > 0)
    {
        cmdValue = command.substring(index + 1);
    }

    if (command.startsWith("SSID"))
    {
        preferences.begin(BOARDNAME, false);
        preferences.putString("ssid", cmdValue.c_str());
        preferences.end();
        storedSSID = cmdValue.c_str();
        returnValue = "1";
    }
    else if (command.startsWith("PASSWORD"))
    {
        preferences.begin(BOARDNAME, false);
        preferences.putString("password", cmdValue.c_str());
        preferences.end();
        storedWifiPassword = cmdValue.c_str();
        returnValue = "2";
    }
    else if (command.startsWith("mqtt_server"))
    {
        preferences.begin(BOARDNAME, false);
        preferences.putString("mqtt_server", cmdValue.c_str());
        preferences.end();
        mqtt_server = cmdValue.c_str();
        returnValue = "3";
    }
    else if (command.startsWith("mqtt_user"))
    {
        preferences.begin(BOARDNAME, false);
        preferences.putString("mqtt_user", cmdValue.c_str());
        preferences.end();
        mqtt_user = cmdValue.c_str();
        returnValue = "4";
    }
    else if (command.startsWith("mqtt_password"))
    {
        preferences.begin(BOARDNAME, false);
        preferences.putString("mqtt_password", cmdValue.c_str());
        preferences.end();
        mqtt_password = cmdValue.c_str();
        returnValue = "5";
    }
    else if (command.startsWith("mqtt_topic"))
    {
        // subscribe to topic
        auto it = mqtt_topics.find(cmdValue.c_str());

        if (it == mqtt_topics.end())
        {
            mqtt_topics[cmdValue.c_str()] = cmdValue.c_str();

            mqttClient.subscribe(cmdValue.c_str());
        }
        // else
        // {
        // }
        returnValue = mqtt_topics[cmdValue.c_str()].c_str();
    }

#if CONFIG_IDF_TARGET_ESP32
    Wire.slaveWrite((uint8_t *)returnValue.c_str(), strlen(returnValue.c_str()));

    Serial.print("Buffer ready with:");
    Serial.println(returnValue.c_str());
#endif
}

void i2c_setup()
{
    // preferences.begin(BOARDNAME, false);
    // mqtt_server = preferences.getString("mqtt_server", "").c_str();
    // mqtt_user = preferences.getString("mqtt_user", "public").c_str();
    // mqtt_password = preferences.getString("mqtt_password", "public").c_str();
    // mqtt_topic = preferences.getString("mqtt_topic", "").c_str();
    //    preferences.end();

    // start i2c
    Wire.begin(121);           // Join I2C bus as the slave with address 121
    Wire.onReceive(onReceive); // When the data transmission is detected call receiveEvent function
    Wire.onRequest(onRequest);
}
