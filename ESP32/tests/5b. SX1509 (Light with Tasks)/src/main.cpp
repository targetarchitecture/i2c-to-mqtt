#include <Arduino.h>
#include <Wire.h>
#include "SN7 pins.h"
#include <SparkFunSX1509.h> // Include SX1509 library
#include <iostream>
#include <string>
#include <vector>

const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 sx1509;                    // Create an SX1509 object to be used throughout

TaskHandle_t i2cTask;
TaskHandle_t LEDs[15];

std::vector<unsigned long> previousMillis;
std::vector<int> LEDpwm;
std::vector<int> previousLEDpwm;

//create i2c Semaphore , and set to useable
SemaphoreHandle_t i2cSemaphore;

void switch_task(void *pvParameters);
void i2c_task(void *pvParameters);

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
      sx1509.pinMode(pin, ANALOG_OUTPUT); // To breathe an LED, make sure you set it as an ANALOG_OUTPUT, so we can PWM the pin
                                          // sx1509.breathe(pin, 1000, 1000, 4000, 4000); // Breathe an LED: 1000ms LOW, 500ms HIGH, 500ms to rise from low to high, 250ms to fall from high to low
    }
  }

  i2cSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(i2cSemaphore);

  for (int pin = 0; pin < 16; pin++)
  {
    LEDpwm.push_back(0);
    previousLEDpwm.push_back(0);
    previousMillis.push_back(millis());
  }

  xTaskCreatePinnedToCore(
      i2c_task,     /* Task function. */
      "i2c Task",   /* name of task. */
      9000,         /* Stack size of task (uxTaskGetStackHighWaterMark: 8200) */
      NULL,         /* parameter of the task */
      2,            /* priority of the task */
      &i2cTask, 1); /* Task handle to keep track of created task */

  for (int pin = 0; pin < 16; pin++)
  {
    xTaskCreatePinnedToCore(
        switch_task,    /* Task function. */
        "Switch Task",  /* name of task. */
        9000,           /* Stack size of task (uxTaskGetStackHighWaterMark: 8200) */
        (void *)pin,    /* parameter of the task */
        1,              /* priority of the task */
        &LEDs[pin], 1); /* Task handle to keep track of created task */
  }
}

void i2c_task(void *pvParameters)
{
  for (;;)
  {
    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    for (int pin = 0; pin < 16; pin++)
    {
      if (previousLEDpwm[pin] != LEDpwm[pin])
      {

        // Call io.analogWrite(<pin>, <0-255>) to configure the
        // PWM duty cycle
        sx1509.pwm(pin, LEDpwm[pin]);
      }
    }
    
    xSemaphoreGive(i2cSemaphore);

    delay(50);
  }
}

void switch_task(void *pvParameters)
{
  /* Inspect our own high water mark on entering the task. */
  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  Serial.print("switch_task uxTaskGetStackHighWaterMark:");
  Serial.println(uxHighWaterMark);

  int *pvParameter;
  pvParameter = (int *)pvParameters;

  int SX1509_LED_PIN;

  memcpy(&SX1509_LED_PIN, &pvParameter, sizeof(int));

  Serial.printf("SX1509_LED_PIN: %i\n", SX1509_LED_PIN);

  for (;;)
  {
    // Ramp brightness up, from 0-255, delay 2ms in between analogWrite's
    for (byte brightness = 0; brightness <= 254; brightness++)
    {
      // Serial.printf("PWM: %i\n", brightness);
      previousLEDpwm[SX1509_LED_PIN] = LEDpwm[SX1509_LED_PIN];
      LEDpwm[SX1509_LED_PIN] = brightness;

      delay(10); // Delay 2 milliseconds
    }

    delay(500); // Delay half-a-second

    // Ramp brightness down, from 255-0, delay 2ms in between analogWrite's
    for (byte brightness = 255; brightness > 0; brightness--)
    {
      // Serial.printf("PWM: %i\n", brightness);

      previousLEDpwm[SX1509_LED_PIN] = LEDpwm[SX1509_LED_PIN];
      LEDpwm[SX1509_LED_PIN] = brightness;

      delay(10); // Delay 2 milliseconds
    }

    delay(500);
  }
}

void loop()
{
  delay(1000);
}
