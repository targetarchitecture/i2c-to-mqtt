#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include "SN7 pins.h"

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup()
{
  Wire.begin(SDA, SCL); //I2C bus

  Serial.begin(115200);

  delay(100);

  Serial.println("PCA9685");

  pwm.begin();
  pwm.reset();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(50); // Analog servos run at ~50 Hz updates

  delay(10); //this is needed as per the spec
}

void loop()
{
  delay(1000);

  // for (size_t pin = 0; pin < 16; pin++)
  // {
size_t pin = 15;


auto pwm2 = random(280, 440);

    pwm.setPWM(pin, 0,pwm2);

    Serial.println(pwm2);
//  }
}

