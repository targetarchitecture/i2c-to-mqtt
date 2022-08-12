#include <Arduino.h>
#include <Wire.h>
#include "SN10 pins.h"
#include <SparkFunSX1509.h> // Include SX1509 library
#include <vector>
#include <sstream>
#include <iostream>

const byte SX1509_ADDRESS = 0x3E; // SX1509 I2C address
SX1509 sx1509;                    // Create an SX1509 object to be used throughout

enum LEDactions
{
  blink,
  breathe,
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
// QueueHandle_t PWM_TXT_Queue;
SemaphoreHandle_t i2cSemaphore;

void blink_LED_task(void *pvParameters);
void fade_LED_task(void *pvParameters);
void i2c_task(void *pvParameters);
void startLED(int pin, LEDactions action);
void stopLED(int pin);
void dealWithAction(std::string msg);
// void addTXTQueue(int SX1509_LED_PIN, int currentPWM);

void setup()
{
  Wire.begin(SDA, SCL); // I2C bus
  Serial.begin(115200); // ESP32 USB Port

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

  // create the 16 task handles
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

  // delay(2000);

  // delay(3200);
  // dealWithAction("Y1,15,5,20");
  // delay(5000);
  dealWithAction("Y1,1");
  dealWithAction("Y1,2");
  dealWithAction("Y1,3");
  dealWithAction("Y1,4");
  dealWithAction("Y1,5");
  dealWithAction("Y1,6");
  dealWithAction("Y1,7");
  dealWithAction("Y1,8");
  dealWithAction("Y1,9");
  dealWithAction("Y1,10");
  dealWithAction("Y1,11");
  dealWithAction("Y1,12");
  dealWithAction("Y1,13");
  dealWithAction("Y1,14");
  dealWithAction("Y1,15");

  // startLED(3, fade);
  // startLED(15, blink);

  // delay(5000);

  // stopLED(3);

  // delay(1000);

  // startLED(3, on);
}

void dealWithAction(std::string msg)
{
  std::vector<std::string> strings;
  std::istringstream f(msg);
  std::string s;

  while (std::getline(f, s, ','))
  {
    strings.push_back(s);

    Serial.println(s.c_str());
  }

  auto action = strings[0].c_str();
  auto pin = atoi(strings[1].c_str());

  if (strcmp(action, "Y1") == 0)
  {
    startLED(pin, blink);
  }
  else if (strcmp(action, "Y2") == 0)
  {
    startLED(pin, breathe);
  }
  else if (strcmp(action, "Y3") == 0)
  {
    // startLED(3, breathe);
  }
}

void i2c_task(void *pvParameters)
{
  for (;;)
  {
    PWM pwm;

    // wait for the next PWM command to be queued - timing is one each loop making it simpler
    xQueueReceive(PWM_Queue, &pwm, portMAX_DELAY);

    // wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    // configure the PWM duty cycle
    sx1509.pwm(pwm.pin, pwm.pwm);

    xSemaphoreGive(i2cSemaphore);

    delay(1);
  }
}

void fade_LED_task(void *pvParameters)
{
  /* Inspect our own high water mark on entering the task. */
  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  Serial.print("fade LED task uxTaskGetStackHighWaterMark:");
  Serial.println(uxHighWaterMark);

  uint32_t SX1509_LED_PIN = 0;
  BaseType_t xResult = xTaskNotifyWait(0X00, 0x00, &SX1509_LED_PIN, portMAX_DELAY);

  // Serial.printf("SX1509_LED_PIN: %i\n", SX1509_LED_PIN);

  for (;;)
  {
    for (int brightness = 255; brightness >= 0; brightness--)
    {
      PWM pwm;
      pwm.pin = SX1509_LED_PIN;
      pwm.pwm = brightness;

      xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);

      delay(2); // Delay 2 milliseconds
    }

    for (int brightness = 0; brightness <= 255; brightness++)
    {

      PWM pwm;
      pwm.pin = SX1509_LED_PIN;
      pwm.pwm = brightness;

      xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);

      delay(2); // Delay 2 milliseconds
    }

    // delay(500);
  }
}

void loop()
{
  delay(1000);
}

void startLED(int pin, LEDactions action)
{
  stopLED(pin);

  if (action == breathe)
  {
    xTaskCreatePinnedToCore(
        fade_LED_task,        /* Task function. */
        "Lighting Fade Task", /* name of task. */
        9000,                 /* Stack size of task (uxTaskGetStackHighWaterMark: 8700) */
        NULL,                 /* parameter of the task */
        1,                    /* priority of the task */
        &LEDtasks[pin],       /* Task handle to keep track of created task */
        1);                   /* Core */

    xTaskNotify(LEDtasks[pin], pin, eSetValueWithOverwrite);
  }
  else if (action == blink)
  {
    xTaskCreatePinnedToCore(
        blink_LED_task,        /* Task function. */
        "Lighting Blink Task", /* name of task. */
        9000,                  /* Stack size of task (uxTaskGetStackHighWaterMark: 8700) */
        NULL,                  /* parameter of the task */
        1,                     /* priority of the task */
        &LEDtasks[pin],        /* Task handle to keep track of created task */
        1);                    /* Core */

    xTaskNotify(LEDtasks[pin], pin, eSetValueWithOverwrite);
  }
  else if (action == on)
  {

    PWM pwm;
    pwm.pin = pin;
    pwm.pwm = 0; // strangely 0 is on with the SX1509

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

  // set PWM to be picked up by the i2c task
  PWM pwm;
  pwm.pin = pin;
  pwm.pwm = 255; // strangely 0 is on with the SX1509

  xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);
}

void blink_LED_task(void *pvParameters)
{
  /* Inspect our own high water mark on entering the task. */
  UBaseType_t uxHighWaterMark;
  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  Serial.print("blink LED task uxTaskGetStackHighWaterMark:");
  Serial.println(uxHighWaterMark);

  uint32_t SX1509_LED_PIN = 0;
  BaseType_t xResult = xTaskNotifyWait(0X00, 0x00, &SX1509_LED_PIN, portMAX_DELAY);

  // Serial.printf("SX1509_LED_PIN: %i\n", SX1509_LED_PIN);

  int currentPWM = 0;

  PWM pwm;
  pwm.pin = SX1509_LED_PIN;
  pwm.pwm = currentPWM; // strangely 0 is on with the SX1509

  xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);

  for (;;)
  {
    if (currentPWM == 0)
    {
      currentPWM = 255;
    }
    else
    {
      currentPWM = 0;
    }

    PWM pwm;
    pwm.pin = SX1509_LED_PIN;
    pwm.pwm = currentPWM;

    xQueueSend(PWM_Queue, &pwm, portMAX_DELAY);

    delay(500); // Delay half-a-second
  }
}
