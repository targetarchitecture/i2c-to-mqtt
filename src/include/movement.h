#pragma once

#include <Wire.h>
#include "messaging.h"
#include <Adafruit_PWMServoDriver.h>
#include "easing.h"
#include "pins.h"

struct servo
{
  uint8_t pin;
  double duration;
  int16_t toDegree;
  int16_t fromDegree;
  int16_t minPulse; // This is the 'minimum' pulse length count (out of 4096) - normally around 100
  int16_t maxPulse; // This is the 'maximum' pulse length count (out of 4096) - normally around 500
  easingCurves easingCurve;
  bool interuptEasing; //https://esp32.com/viewtopic.php?t=10855;
  TaskHandle_t taskHandle;
};

struct servoPWM
{
  uint16_t pwm;
  uint8_t pin;
};

void ServoEasingTaskV3(void *pvParameter);
double mapAngles(const uint16_t x, const uint16_t in_min,const  uint16_t in_max, const uint16_t out_min, const uint16_t out_max);
void setServoAngle(const uint8_t pin, const uint16_t angle, const uint16_t minPulse, const uint16_t maxPulse);
void stopServo(const uint8_t pin);
void setServoPWM(const uint8_t pin, const uint16_t PWM);
void setServoEase(const uint8_t pin, easingCurves easingCurve, const uint16_t toDegree, const uint16_t fromDegree, const uint16_t duration, const uint16_t minPulse, const uint16_t maxPulse);
void movement_setup();
void movement_task(void *pvParameter);
void movement_i2c_task(void *pvParameter);

extern void checkI2Cerrors(std::string area);
extern void POST(uint8_t flashes);

extern QueueHandle_t Movement_Queue;
extern SemaphoreHandle_t i2cSemaphore;
