#pragma once

#include <Wire.h>
#include "messaging.h"
#include "defines.h"
#include "pins.h"
#include <SparkFunSX1509.h> // Include SX1509 library

void light_setup();
void light_task(void *pvParameter);
void TurnLEDOnOff(uint32_t pin, int OnOff);
void stopCurrentTaskOnPin(uint32_t pin);

void LEDBlinkingTask(void *pvParameter);
void LEDBreathingTask(void *pvParameter);

extern void POST(uint8_t flashes);
extern void checkI2Cerrors(std::string area);

extern QueueHandle_t Light_Queue;
extern SemaphoreHandle_t i2cSemaphore;

enum pinState
{
    on,
    off,
    blink,
    breathe
};

struct LED
{
    uint16_t pin;
    pinState state;
    uint32_t OnTimeMS;
    uint32_t OffTimeMS;
    uint32_t RiseTimeMS;
    uint32_t FallTimeMS;
    TaskHandle_t taskHandle;
};

//LED
#define LEDON 0
#define LEDOFF 255