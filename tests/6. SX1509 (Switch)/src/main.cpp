#include <Arduino.h>
#include <Wire.h>
#include "SN4 pins.h"
#include <SparkFunSX1509.h> // Include SX1509 library

const byte SX1509_ADDRESS = 0x3F; // SX1509 I2C address
SX1509 switchboard;               // Create an SX1509 object to be used throughout
int SX1509_BTN_PIN = 15;

void setup()
{
  Wire.begin(SDA,SCL);         //I2C bus
  Serial.begin(115200); //ESP32 USB Port

  if (!switchboard.begin(SX1509_ADDRESS))
  {
    Serial.println("SX1509 not found");
  }
  else
  {
    Serial.println("SX1509 lighting found");

    // Use the internal 2MHz oscillator.
    switchboard.clock(INTERNAL_CLOCK_2MHZ, 4);

    switchboard.pinMode(SX1509_BTN_PIN, INPUT_PULLUP);
  }

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  Serial.print("button state:");
  Serial.println( switchboard.digitalRead(SX1509_BTN_PIN));

  digitalWrite(LED_BUILTIN, switchboard.digitalRead(SX1509_BTN_PIN));

  delay(500);
}
