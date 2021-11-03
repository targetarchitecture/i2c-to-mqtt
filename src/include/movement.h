#pragma once

#include <Wire.h>
#include "messaging.h"
#include <Adafruit_PWMServoDriver.h>
#include "easing.h"
#include "SN8 pins.h"
#include <Streaming.h>

struct servo
{
  int16_t pin;
  //int16_t PWM;
  //double _change;
  double duration;
  int16_t toDegree;
  int16_t fromDegree;
  //int16_t setDegree;
  int16_t minPulse; // This is the 'minimum' pulse length count (out of 4096) - normally around 100
  int16_t maxPulse; // This is the 'maximum' pulse length count (out of 4096) - normally around 500
  //bool isMoving;
  easingCurves easingCurve;
  bool interuptEasing; //https://esp32.com/viewtopic.php?t=10855;
  TaskHandle_t taskHandle;
};

struct servoPWM
{
  int pwm;
  int pin;
};

void ServoEasingTaskV2(void *pvParameter);
void ServoEasingTaskV1(void *pvParameter);
double mapAngles(double x, double in_min, double in_max, double out_min, double out_max);
void setServoAngle(const int16_t pin, const int16_t angle, const int16_t minPulse, const int16_t maxPulse);
void stopServo(const int16_t pin);
void setServoPWM(const int16_t pin, const int16_t PWM);
void setServoEase(const int16_t pin, easingCurves easingCurve, const int16_t toDegree, const int16_t fromDegree, const int16_t duration, const int16_t minPulse, const int16_t maxPulse);
void movement_setup();
void movement_task(void *pvParameter);
void movement_i2c_task(void *pvParameter);

extern void checkI2Cerrors(std::string area);
extern void POST(uint8_t flashes);

extern QueueHandle_t Movement_Queue;
extern SemaphoreHandle_t i2cSemaphore;
