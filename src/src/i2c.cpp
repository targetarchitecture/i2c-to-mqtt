#include <Arduino.h>
#include "i2c.h"
#include "defines.h"

void ReceivedCommand(int howMany)
{
    String command;
    String cmdValue = "";

    Serial.println("ReceivedCommand:");

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
        preferences.putString("ssid", cmdValue.c_str());
    }

    else if (command.startsWith("PASSWORD"))
    {
        preferences.putString("password", cmdValue.c_str());
    }

    else if (command.startsWith("mqtt_server"))
    {
        preferences.putString("mqtt_server", cmdValue.c_str());
    }

    else if (command.startsWith("mqtt_user"))
    {
        preferences.putString("mqtt_user", cmdValue.c_str());
    }

    else if (command.startsWith("mqtt_password"))
    {
        preferences.putString("mqtt_password", cmdValue.c_str());
    }

    else if (command.startsWith("mqtt_topic"))
    {
        preferences.putString("mqtt_topic", cmdValue.c_str());
    }
}

void SendData()
{
}

void i2c_setup()
{
    // start i2c
    Wire.begin(121);                 // Join I2C bus as the slave with address 121
    Wire.onReceive(ReceivedCommand); // When the data transmission is detected call receiveEvent function
    Wire.onRequest(SendData);
}