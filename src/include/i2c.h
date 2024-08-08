#pragma once

#include <Wire.h>
#include <Preferences.h>

void ReceivedCommand(int);
void SendData();
void i2c_setup();

extern Preferences preferences;