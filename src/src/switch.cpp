#include <Arduino.h>
#include "switch.h"
#include <SparkFunSX1509.h> // Include SX1509 library

SX1509 switches; // Create an SX1509 object to be used throughout

TaskHandle_t SwitchTask;

volatile int switchArray[16] = {};

void switch_setup()
{
    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    if (!switches.begin(0x3F))
    {
        Serial.println("SX1509 for switching not found");

        POST(2);
    }

    // Use the internal 2MHz oscillator.
    //switches.clock(INTERNAL_CLOCK_2MHZ,4);  //TODO: Review the ,4 I've just added
    switches.debounceTime(32); //64

    for (size_t i = 0; i < 16; i++)
    {
        switches.pinMode(i, INPUT_PULLUP);
        switches.debouncePin(i);
        switches.enableInterrupt(i, CHANGE); //instruct the pin to set the interupt pin

        //Serial.print("debouncePin:");
        //Serial.println(i);
    }

    checkI2Cerrors("switch");

    //give back the i2c flag for the next task
    xSemaphoreGive(i2cSemaphore);

    xTaskCreatePinnedToCore(
        switch_task,          /* Task function. */
        "Switch Task",        /* name of task. */
        8500,                 /* Stack size of task (uxTaskGetStackHighWaterMark: 8204)   */
        NULL,                 /* parameter of the task */
        switch_task_Priority, /* priority of the task */
        &SwitchTask, 1);      /* Task handle to keep track of created task */

    //set-up the input pin
    pinMode(SWITCH_INT, INPUT_PULLUP);
}

void switch_task(void *pvParameters)
{
    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("switch_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    BaseType_t xResult;

    for (;;)
    {
        //On SN7 there will be an wait for interupt here to prevent scanning if there's no event occured
        if (digitalRead(SWITCH_INT) == LOW)
        {
            //quickly read all of the pins and save the state

            //wait for the i2c semaphore flag to become available
            xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

            checkI2Cerrors("switch (switch_task start)");

            //quickly read all of the pins and save the state
            for (size_t i = 0; i < 16; i++)
            {
                switchArray[i] = switches.digitalRead(i);
            }

            checkI2Cerrors("switch (switch_task end)");

            //give back the i2c flag for the next task
            xSemaphoreGive(i2cSemaphore);
        }

        // put a delay so it isn't overwhelming
        delay(50);
    }

    vTaskDelete(NULL);
}
