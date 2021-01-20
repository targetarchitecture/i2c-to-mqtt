#include <Arduino.h>
#include <Wire.h>
#include "SN7 pins.h"
#include <SparkFunSX1509.h> // Include SX1509 library

const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 sx1509;                    // Create an SX1509 object to be used throughout

void setup()
{
  Wire.begin(SDA, SCL); //I2C bus
  Serial.begin(115200); //ESP32 USB Port

  if (!sx1509.begin(SX1509_ADDRESS))
  {
    Serial.println("SX1509 not found");
  }
  else
  {
    Serial.println("SX1509 lighting found");

    sx1509.clock(INTERNAL_CLOCK_2MHZ, 4);

    for (size_t pin = 0; pin < 16; pin++)
    {
      sx1509.pinMode(pin, ANALOG_OUTPUT);          // To breathe an LED, make sure you set it as an ANALOG_OUTPUT, so we can PWM the pin
      sx1509.breathe(pin, 1000, 1000, 4000, 4000); // Breathe an LED: 1000ms LOW, 500ms HIGH, 500ms to rise from low to high, 250ms to fall from high to low
    }
  }
}

void loop()
{
  delay(1000);
}
