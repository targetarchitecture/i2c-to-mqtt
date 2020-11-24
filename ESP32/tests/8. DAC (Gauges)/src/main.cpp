#include <Arduino.h>
#include "SN4 pins.h"
//https://www.xtronical.com/basics/audio/dacs-on-esp32/

int SineValues[256]; // an array to store our values for sine
float ConversionFactor = (2 * PI) / 256;
float RadAngle;

void setup()
{
  // calculate sine values
  for (int MyAngle = 0; MyAngle < 256; MyAngle++)
  {
    RadAngle = MyAngle * ConversionFactor;             // 8 bit angle converted to radians
    SineValues[MyAngle] = (sin(RadAngle) * 127) + 128; // get the sine of this angle and 'shift' up so
                                                       // there are no negative values in the data
                                                       // as the DAC does not understand them and would
                                                       // convert to positive values.
  }
}

void loop()
{
  for (int i = 0; i < 256; i++)
  {
    dacWrite(DAC1, SineValues[i]);
    dacWrite(DAC2, SineValues[i]);
    delay(5);
  }
}
