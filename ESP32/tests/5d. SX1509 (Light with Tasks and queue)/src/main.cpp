#include <Arduino.h>
#include <Wire.h>
#include "SN7 pins.h"
#include <SparkFunSX1509.h> // Include SX1509 library
//#include <iostream>
//#include <string>
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

struct PWM
{
  int pwm;
  int pin;
};

TaskHandle_t i2cTask;
std::vector<TaskHandle_t> LEDtasks;
QueueHandle_t PWM_Queue;
SemaphoreHandle_t i2cSemaphore;

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
      sx1509.pinMode(pin, ANALOG_OUTPUT);
    }
  }

  xSemaphoreGive(i2cSemaphore);

//create the 16 task handles
  for (int pin = 0; pin < 16; pin++)
  {
    LEDtasks.push_back(NULL);
  }

  PWM_Queue = xQueueCreate(100, sizeof(PWM));

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
  startLED(15, blink);

  delay(5000);

  stopLED(3);

  delay(1000);

  startLED(3,on);
}

void i2c_task(void *pvParameters)
{
  for (;;)
  {
    PWM pwm;

    //wait for the next PWM command to be queued - timing is one each loop making it simpler
    xQueueReceive(PWM_Queue, &pwm, portMAX_DELAY);

    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    // configure the PWM duty cycle
    sx1509.pwm(pwm.pin, pwm.pwm);

    xSemaphoreGive(i2cSemaphore);
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

  PWM pwm;
  pwm.pin = SX1509_LED_PIN;
  pwm.pwm = 0;

  xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);

  for (;;)
  {
    // Ramp brightness up, from 0-255, delay 2ms in between analogWrite's
    for (int brightness = 0; brightness <= 255; brightness++)
    {
      // Serial.printf("PWM: %i\n", brightness);

      pwm.pwm = brightness;

      xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);

      delay(10); // Delay 2 milliseconds
    }

    pwm.pwm = 255;

    xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);

    delay(500); // Delay half-a-second

    // Ramp brightness down, from 255-0, delay 2ms in between analogWrite's
    for (int brightness = 255; brightness >= 0; brightness--)
    {
      pwm.pwm = brightness;

      xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);

      delay(10); // Delay 2 milliseconds
    }

    pwm.pwm = 0;

    xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);

    delay(500);
  }
}

void loop()
{
  delay(1000);
}

void startLED(int pin, LEDactions action)
{
  switch (action)
  {
  case fade:
    xTaskCreatePinnedToCore(
        fade_LED_task,        /* Task function. */
        "Lighting Fade Task", /* name of task. */
        9000,                 /* Stack size of task (uxTaskGetStackHighWaterMark: 8200) */
        (void *)pin,          /* parameter of the task */
        1,                    /* priority of the task */
        &LEDtasks[pin],           /* Task handle to keep track of created task */
        1);                   /* Core */

  case blink:
    xTaskCreatePinnedToCore(
        blink_LED_task,        /* Task function. */
        "Lighting Blink Task", /* name of task. */
        9000,                  /* Stack size of task (uxTaskGetStackHighWaterMark: 8200) */
        (void *)pin,           /* parameter of the task */
        1,                     /* priority of the task */
        &LEDtasks[pin],            /* Task handle to keep track of created task */
        1);                    /* Core */

  case on:
    PWM pwm;
    pwm.pin = pin;
    pwm.pwm = 0; //strangely 0 is on with the SX1509

    xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);
  }
}

void stopLED(int pin)
{
  Serial.printf("STOPPING SX1509_LED_PIN: %i\n", pin);

  TaskHandle_t xTask = LEDtasks[pin];

  vTaskSuspendAll();

  if (LEDtasks[pin] != NULL)
  {
    /* The task is going to be deleted. Set the handle to NULL. */
    LEDtasks[pin] = NULL;

    /* Delete using the copy of the handle. */
    vTaskDelete(xTask);
  }

  xTaskResumeAll();

  //set PWM to be picked up by the i2c task
  PWM pwm;
  pwm.pin = pin;
  pwm.pwm = 0; //strangely 0 is on with the SX1509

  xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);
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

  PWM pwm;
  pwm.pin = SX1509_LED_PIN;
  pwm.pwm = 0; //strangely 0 is on with the SX1509

  xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);

  for (;;)
  {
    if (pwm.pwm == 0)
    {
      pwm.pwm = 255;
    }
    else
    {
      pwm.pwm = 0;
    }

    xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);

    delay(500); // Delay half-a-second
  }
}
