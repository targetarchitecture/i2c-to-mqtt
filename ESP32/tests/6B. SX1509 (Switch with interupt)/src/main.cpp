#include <Arduino.h>
#include <Wire.h>
#include "SN7 pins.h"
#include <SparkFunSX1509.h> // Include SX1509 library

TaskHandle_t SwitchTask;
int pinState[15]; // {HIGH};

const byte SX1509_ADDRESS = 0x3F; // SX1509 I2C address
SX1509 switches;                  // Create an SX1509 object to be used throughout
int SX1509_BTN_PIN = 15;

void switch_task(void *pvParameters);
void IRAM_ATTR handleSwitchInterupt();

void setup()
{
  Wire.begin(SDA, SCL); //I2C bus
  Serial.begin(115200); //ESP32 USB Port

  if (!switches.begin(SX1509_ADDRESS))
  {
    Serial.println("SX1509 not found");
  }
  else
  {
    Serial.println("SX1509 switch found");

    // Use the internal 2MHz oscillator.
    switches.clock(INTERNAL_CLOCK_2MHZ, 4);

    switches.pinMode(SX1509_BTN_PIN, INPUT_PULLUP);

    xTaskCreatePinnedToCore(
        switch_task,     /* Task function. */
        "Switch Task",   /* name of task. */
        8500,            /* Stack size of task (uxTaskGetStackHighWaterMark: 7728) */
        NULL,            /* parameter of the task */
        1,               /* priority of the task */
        &SwitchTask, 1); /* Task handle to keep track of created task */

    //set-up the interupt
    pinMode(SWITCH_INT, INPUT_PULLUP);
    //attachInterrupt(SWITCH_INT, handleSwitchInterupt, CHANGE);
  }
}

void switch_task(void *pvParameters)
{

  //TODO: Ask Google if this is the best place to declare variables in an endless task
  byte newPinState[16]; // {HIGH};

  for (size_t i = 0; i < 16; i++)
  {
    //set the pin states to HIGH as PULLUP is set
    pinState[i] = 1;    //HIGH;
    newPinState[i] = 1; // HIGH;

    switches.pinMode(i, INPUT_PULLUP);
    switches.enableInterrupt(i, CHANGE);
  }

  //TODO: see if this improves the inital flood of readings
  //delay(100);

  uint32_t ulNotifiedValue = 0;
  BaseType_t xResult;

  /* Inspect our own high water mark on entering the task. */
//   UBaseType_t uxHighWaterMark;
//   uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
//   Serial.print("switch_task uxTaskGetStackHighWaterMark:");
//   Serial.println(uxHighWaterMark);

// delay(1000);

  //   Serial.printf("Switch task is on core %i\n", xPortGetCoreID());

  for (;;)
  {
    // try
    // {
    /* code */

    // Serial.println("waiting for xTaskNotifyWait");

    // //TODO: On SN7 there will be an wait for interupt here to prevent scanning if there's no event occured
    // xResult = xTaskNotifyWait(0X00, 0x00, &ulNotifiedValue, portMAX_DELAY);

    // Serial.println("proceeding after xTaskNotifyWait");

    if (digitalRead(SWITCH_INT) == LOW)
    {
      Serial.println("digitalRead(SWITCH_INT) == LOW");
      // delay(1);

      for (size_t i = 0; i < 16; i++)
      {
        newPinState[i] = switches.digitalRead(i);
      }

      // for (size_t i = 0; i < 16; i++)
      // {
      //   //check for differance
      //   if (pinState[i] != newPinState[i])
      //   {
      //     Serial.printf("Readings (%i): %i,%i \n", i, pinState[i], newPinState[i]);

      //     pinState[i] = newPinState[i];
      //   }
      // }
      // }
      // catch (const std::exception &e)
      // {
      //   // std::cerr << e.what() << '\n';
      //   Serial.println(e.what());
      // }

 delay(50);

    }
  }

  vTaskDelete(NULL);
}

void loop()
{
  // Serial.print("button state:");
  // Serial.println( switchboard.digitalRead(SX1509_BTN_PIN));

  // digitalWrite(LED_BUILTIN, switchboard.digitalRead(SX1509_BTN_PIN));

  delay(1);
}

//volatile i

void IRAM_ATTR handleSwitchInterupt()
{
  xTaskNotify(SwitchTask, 0, eSetValueWithoutOverwrite);

  // try
  // {
  //   /* code */
  //   BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  //   //int32_t cmd = 1;
  //   xTaskNotifyFromISR(SwitchTask, 0, eSetBits, &xHigherPriorityTaskWoken);
  //   //xTaskNotify(SwitchTask, 0, eSetValueWithoutOverwrite);
  // }
  // catch (const std::exception &e)
  // {
  //   // std::cerr << e.what() << '\n';
  //   Serial.println(e.what());
  // }
}