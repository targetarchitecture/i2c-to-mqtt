#include <Arduino.h>
#include <Wire.h>
#include "SN7 pins.h"
#include <SparkFunSX1509.h> // Include SX1509 library
#include <iostream>
#include <string>
#include <vector>

const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 sx1509;                    // Create an SX1509 object to be used throughout

TaskHandle_t lightTask;

enum LEDaction
{
  blink,
  fade,
  off,
  on
};

std::vector<unsigned long> previousMillis;
std::vector<int> LEDpwm;        //= {255};
std::vector<long> LEDintervals; // = {600};
std::vector<LEDaction> state;   // = {blink};

//create i2c Semaphore , and set to useable
SemaphoreHandle_t i2cSemaphore;

//void switch_task(void *pvParameters);
//void i2c_task(void *pvParameters);

void setup()
{
  Wire.begin(SDA, SCL); //I2C bus
  Serial.begin(115200); //ESP32 USB Port

  if (!sx1509.begin(SX1509_ADDRESS))
  {
    Serial.println("SX1509 not found");
    delay(10000);
  }
  else
  {
    Serial.println("SX1509 lighting found");

    //sx1509.clock(INTERNAL_CLOCK_2MHZ, 4);

    for (size_t pin = 0; pin < 16; pin++)
    {
      sx1509.pinMode(pin, ANALOG_OUTPUT); // To breathe an LED, make sure you set it as an ANALOG_OUTPUT, so we can PWM the pin
                                          // sx1509.breathe(pin, 1000, 1000, 4000, 4000); // Breathe an LED: 1000ms LOW, 500ms HIGH, 500ms to rise from low to high, 250ms to fall from high to low
    }
  }

  // i2cSemaphore = xSemaphoreCreateBinary();
  // xSemaphoreGive(i2cSemaphore);

  for (int pin = 0; pin < 16; pin++)
  {
    LEDpwm.push_back(0);
    previousMillis.push_back(millis());
    LEDintervals.push_back(1000);
    state.push_back(blink);
  }

  // xTaskCreatePinnedToCore(
  //     i2c_task,       /* Task function. */
  //     "Light Task",   /* name of task. */
  //     9000,           /* Stack size of task (uxTaskGetStackHighWaterMark: 8200) */
  //     NULL,           /* parameter of the task */
  //     2,              /* priority of the task */
  //     &lightTask, 1); /* Task handle to keep track of created task */
}

// void i2c_task(void *pvParameters)
// {
//   /* Inspect our own high water mark on entering the task. */
//   UBaseType_t uxHighWaterMark;
//   uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
//   Serial.print("Light uxTaskGetStackHighWaterMark:");
//   Serial.println(uxHighWaterMark);

//   for (;;)
//   {
//   }

//   // void switch_task(void *pvParameters)
//   // {

//   //   int *pvParameter;
//   //   pvParameter = (int *)pvParameters;

//   //   int SX1509_LED_PIN;

//   //   memcpy(&SX1509_LED_PIN, &pvParameter, sizeof(int));

//   //   Serial.printf("SX1509_LED_PIN: %i\n", SX1509_LED_PIN);

//   //   for (;;)
//   //   {

//   //     // Ramp brightness up, from 0-255, delay 2ms in between
//   //     // analogWrite's
//   //     for (byte brightness = 0; brightness <= 254; brightness++)
//   //     {
//   //       // Serial.printf("PWM: %i\n", brightness);

//   //       X[SX1509_LED_PIN] = brightness;

//   //       delay(10); // Delay 2 milliseconds
//   //     }

//   //     // delay(500); // Delay half-a-second

//   //     // Ramp brightness down, from 255-0, delay 2ms in between
//   //     // analogWrite's
//   //     for (byte brightness = 255; brightness > 0; brightness--)
//   //     {
//   //       // Serial.printf("PWM: %i\n", brightness);

//   //       X[SX1509_LED_PIN] = brightness;

//   //       //wait for the i2c semaphore flag to become available
//   //       // xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

//   //       // sx1509.pwm(SX1509_LED_PIN, brightness);

//   //       // xSemaphoreGive(i2cSemaphore);

//   //       delay(10); // Delay 2 milliseconds
//   //     }

//   //     // delay(500);
//   //   }
// }

void loop()
{
  // delay(1000);

  unsigned long currentMillis = millis();

  for (int pin = 0; pin < 16; pin++)
  {
    // auto A = currentMillis - previousMillis[pin];
    // auto B = LEDintervals[pin];

    if ((currentMillis - previousMillis[pin]) >= LEDintervals[pin])
    {
      Serial.println(currentMillis - previousMillis[pin]);
      Serial.println(LEDintervals[pin]);

      previousMillis[pin] = currentMillis;

      switch (state[pin])
      {
      case blink:
        if (LEDpwm[pin] == 0)
        {
          LEDpwm[pin] = 255;
        }
        else
        {
          LEDpwm[pin] = 0;
        }

        Serial.print("Pin ");
        Serial.print(pin);
        Serial.print(" PWM");
        Serial.println(LEDpwm[pin]);

        sx1509.analogWrite(pin, LEDpwm[pin]);
      }
    }
  }

  //wait for the i2c semaphore flag to become available
  // xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

  // for (int pin = 0; pin < 16; pin++)
  // {
  //     Serial.println(LEDpwm[pin]);

  //   sx1509.pwm(pin, LEDpwm[pin]);
  //   delay(2);
  // }

  // xSemaphoreGive(i2cSemaphore);

  delay(100); // Delay 2 milliseconds
}