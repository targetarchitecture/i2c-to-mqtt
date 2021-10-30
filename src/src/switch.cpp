#include <Arduino.h>
#include "switch.h"
#include <SparkFunSX1509.h> // Include SX1509 library

SX1509 switches; // Create an SX1509 object to be used throughout

TaskHandle_t SwitchTask;

volatile byte switchArray[16] = {};

void switch_setup()
{
    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    if (!switches.begin(0x3F))
    {
        Serial.println("SX1509 for switching not found");

        POST(2);
    }

    for (size_t i = 0; i < 16; i++)
    {
        switches.pinMode(i, INPUT_PULLUP);
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
        // put a delay so it isn't overwhelming
        delay(100);
    }

    vTaskDelete(NULL);
}

//function to set the device and set the array
uint16_t readAndSetArray()
{
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