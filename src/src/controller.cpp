#include <Arduino.h>
#include "controller.h"

void controller_setup()
{

   // Ps3.attach(notify);
   //Ps3.attachOnConnect(onConnect);
    Ps3.begin();

    //String address = Ps3.getAddress();

    // Serial.print("The ESP32's Bluetooth MAC address is: ");
    // Serial.println(address);

    // Ps3.begin("98:f4:ab:67:8e:8e");
    //Ps3.begin(address.c_str());

    // delay(100);
}

void onConnect()
{
    Serial.println("PS3 Connected!.");
}

void notify()
{
    if (Ps3.data.button.cross)
    {
        Serial.println("Pressing the cross button");
    }

    if (Ps3.data.button.square)
    {
        Serial.println("Pressing the square button");
    }

    if (Ps3.data.button.triangle)
    {
        Serial.println("Pressing the triangle button");
    }

    if (Ps3.data.button.circle)
    {
        Serial.println("Pressing the circle button");
    }
}
