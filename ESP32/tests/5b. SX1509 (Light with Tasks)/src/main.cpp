#include <Arduino.h>
#include <Wire.h>
#include "SN7 pins.h"
#include <SparkFunSX1509.h> // Include SX1509 library
#include <iostream>
#include <string>
#include <vector>

const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 sx1509;                    // Create an SX1509 object to be used throughout

enum LEDactions
{
  blink,
  fade,
  off,
  on
};

TaskHandle_t i2cTask;

std::vector<unsigned long> previousMillis;
std::vector<int> LEDpwm;
std::vector<int> previousLEDpwm;
std::vector<TaskHandle_t> LEDs;
std::vector<LEDactions> LEDstate;
std::vector<SemaphoreHandle_t> pwmSemaphore;

//create i2c Semaphore , and set to useable
SemaphoreHandle_t i2cSemaphore;
//SemaphoreHandle_t pwmSemaphore;

void blink_LED_task(void *pvParameters);
void fade_LED_task(void *pvParameters);
void i2c_task(void *pvParameters);
void startLED(int pin, LEDactions action);
void stopLED(int pin);

void setup()
{
  Wire.begin(SDA, SCL); //I2C bus
  Serial.begin(115200); //ESP32 USB Port

  i2cSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(i2cSemaphore);
  xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

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
      sx1509.pinMode(pin, ANALOG_OUTPUT); // To breathe an LED, make sure you set it as an ANALOG_OUTPUT, so we can PWM the pin
                                          // sx1509.breathe(pin, 1000, 1000, 4000, 4000); // Breathe an LED: 1000ms LOW, 500ms HIGH, 500ms to rise from low to high, 250ms to fall from high to low
    }
  }

  xSemaphoreGive(i2cSemaphore);

  for (int pin = 0; pin < 16; pin++)
  {
    LEDpwm.push_back(0);
    previousLEDpwm.push_back(0);
    previousMillis.push_back(millis());
    LEDs.push_back(NULL);
    LEDstate.push_back(off);

    pwmSemaphore.push_back(xSemaphoreCreateBinary());
  }

  for (int pin = 0; pin < 16; pin++)
  {
    xSemaphoreGive(pwmSemaphore[pin]);
  }

  xTaskCreatePinnedToCore(
      i2c_task,   /* Task function. */
      "i2c Task", /* name of task. */
      9000,       /* Stack size of task (uxTaskGetStackHighWaterMark: 8200) */
      NULL,       /* parameter of the task */
      2,          /* priority of the task */
      &i2cTask,   /* Task handle to keep track of created task */
      1);         /*core! */

  delay(2000);

  startLED(3, fade);
  //startLED(15, blink);

  // delay(5000);

  // stopLED(3);

  // delay(1000);

  // startLED(3,on);
}

void i2c_task(void *pvParameters)
{
  for (;;)
  {
    for (int pin = 0; pin < 16; pin++)
    {
      if (LEDs[pin] != NULL)
      {
        xSemaphoreTake(pwmSemaphore[pin], portMAX_DELAY);

        int PWM = LEDpwm[pin];

        xSemaphoreGive(pwmSemaphore[pin]);

        if (previousLEDpwm[pin] != PWM)
        {
          //wait for the i2c semaphore flag to become available
          xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

          // Call io.analogWrite(<pin>, <0-255>) to configure the
          // PWM duty cycle
          sx1509.pwm(pin, PWM);

          xSemaphoreGive(i2cSemaphore);

          if (pin == 3)
          {
            Serial.println(PWM);
          }

          //now it's changed set the previous PWM value
          previousLEDpwm[pin] = PWM;
        }
      }
    }

    delay(10);
  }
}

void fade_LED_task(void *pvParameters)
{
  /* Inspect our own high water mark on entering the task. */
  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  Serial.print("fade LED task uxTaskGetStackHighWaterMark:");
  Serial.println(uxHighWaterMark);

  int *pvParameter;
  pvParameter = (int *)pvParameters;

  int SX1509_LED_PIN;

  memcpy(&SX1509_LED_PIN, &pvParameter, sizeof(int));

  Serial.printf("SX1509_LED_PIN: %i\n", SX1509_LED_PIN);

  LEDstate[SX1509_LED_PIN] = fade;

  bool printed = false;

  xSemaphoreTake(pwmSemaphore[SX1509_LED_PIN], portMAX_DELAY);

  LEDpwm[SX1509_LED_PIN] = 0;

  xSemaphoreGive(pwmSemaphore[SX1509_LED_PIN]);

  for (;;)
  {
    // Ramp brightness up, from 0-255, delay 2ms in between analogWrite's
    for (int brightness = 0; brightness <= 255; brightness++)
    {
      // Serial.printf("PWM: %i\n", brightness);

      xSemaphoreTake(pwmSemaphore[SX1509_LED_PIN], portMAX_DELAY);

      LEDpwm[SX1509_LED_PIN] = brightness;

      xSemaphoreGive(pwmSemaphore[SX1509_LED_PIN]);

      //previousLEDpwm[SX1509_LED_PIN] = LEDpwm[SX1509_LED_PIN];

      delay(10); // Delay 2 milliseconds
    }

    xSemaphoreTake(pwmSemaphore[SX1509_LED_PIN], portMAX_DELAY);

    LEDpwm[SX1509_LED_PIN] = 255;

    xSemaphoreGive(pwmSemaphore[SX1509_LED_PIN]);

    delay(500); // Delay half-a-second

    // Ramp brightness down, from 255-0, delay 2ms in between analogWrite's
    for (int brightness = 255; brightness >= 0; brightness--)
    {
      // Serial.printf("PWM: %i\n", brightness);

      //previousLEDpwm[SX1509_LED_PIN] = LEDpwm[SX1509_LED_PIN];

      xSemaphoreTake(pwmSemaphore[SX1509_LED_PIN], portMAX_DELAY);

      LEDpwm[SX1509_LED_PIN] = brightness;

      xSemaphoreGive(pwmSemaphore[SX1509_LED_PIN]);

      if (printed == false)
      {
        Serial.println(LEDpwm[SX1509_LED_PIN]);
      }

      delay(10); // Delay 2 milliseconds
    }

    xSemaphoreTake(pwmSemaphore[SX1509_LED_PIN], portMAX_DELAY);

    LEDpwm[SX1509_LED_PIN] = 0;

    xSemaphoreGive(pwmSemaphore[SX1509_LED_PIN]);

    //   LEDpwm[SX1509_LED_PIN] = 0;
    delay(500);

    printed = true;
  }
}

void loop()
{
  delay(1000);
}

void startLED(int pin, LEDactions action)
{
  LEDpwm[pin] = 255; // 255 is OFF
  previousLEDpwm[pin] = -1;
  previousMillis[pin] = millis();
  LEDstate[pin] = action;

  switch (action)
  {
  case fade:
    xTaskCreatePinnedToCore(
        fade_LED_task,        /* Task function. */
        "Lighting Fade Task", /* name of task. */
        9000,                 /* Stack size of task (uxTaskGetStackHighWaterMark: 8200) */
        (void *)pin,          /* parameter of the task */
        1,                    /* priority of the task */
        &LEDs[pin],           /* Task handle to keep track of created task */
        1);                   /* Core */
  case blink:
    xTaskCreatePinnedToCore(
        blink_LED_task,        /* Task function. */
        "Lighting Blink Task", /* name of task. */
        9000,                  /* Stack size of task (uxTaskGetStackHighWaterMark: 8200) */
        (void *)pin,           /* parameter of the task */
        1,                     /* priority of the task */
        &LEDs[pin],            /* Task handle to keep track of created task */
        1);                    /* Core */
  case on:
    LEDstate[pin] = on;
    LEDpwm[pin] = 0; //strangely 0 is on with the SX1509
  }
}

void stopLED(int pin)
{
  Serial.printf("STOPPING SX1509_LED_PIN: %i\n", pin);

  TaskHandle_t xTask = LEDs[pin];

  vTaskSuspendAll();

  if (LEDs[pin] != NULL)
  {
    /* The task is going to be deleted. Set the handle to NULL. */
    LEDs[pin] = NULL;

    /* Delete using the copy of the handle. */
    vTaskDelete(xTask);
  }

  xTaskResumeAll();

  //set PWM to be picked up by the i2c task
  previousLEDpwm[pin] = 254;
  LEDpwm[pin] = 255;
  LEDstate[pin] = off;
}

void blink_LED_task(void *pvParameters)
{
  /* Inspect our own high water mark on entering the task. */
  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  Serial.print("blink LED task uxTaskGetStackHighWaterMark:");
  Serial.println(uxHighWaterMark);

  int *pvParameter;
  pvParameter = (int *)pvParameters;

  int SX1509_LED_PIN;

  memcpy(&SX1509_LED_PIN, &pvParameter, sizeof(int));

  Serial.printf("SX1509_LED_PIN: %i\n", SX1509_LED_PIN);

  LEDstate[SX1509_LED_PIN] = blink;

  for (;;)
  {
    if (LEDpwm[SX1509_LED_PIN] == 0)
    {

      xSemaphoreTake(pwmSemaphore[SX1509_LED_PIN], portMAX_DELAY);

      LEDpwm[SX1509_LED_PIN] = 255;

      xSemaphoreGive(pwmSemaphore[SX1509_LED_PIN]);

      // LEDpwm[SX1509_LED_PIN] = 255;
    }
    else
    {

      xSemaphoreTake(pwmSemaphore[SX1509_LED_PIN], portMAX_DELAY);

      LEDpwm[SX1509_LED_PIN] = 0;

      xSemaphoreGive(pwmSemaphore[SX1509_LED_PIN]);

      //  LEDpwm[SX1509_LED_PIN] = 0;
    }

    delay(500); // Delay half-a-second
  }
}
