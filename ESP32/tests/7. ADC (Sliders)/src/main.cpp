#include <Arduino.h>
#include "SN4 pins.h"
//https://microcontrollerslab.com/adc-esp32-measuring-voltage-example/

void setup()
{
  Serial.begin(115200); //ESP32 USB Port

  adcAttachPin(ADC1);
  adcAttachPin(ADC2);
}

void loop()
{
  delay(200);

  Serial.print("ADC1 value:");
  Serial.print(map(analogRead(ADC1), 0, 4095, 0, 100));
  Serial.print("\tADC2 value:");
  Serial.println(map(analogRead(ADC2), 0, 4095, 0, 100));
}
