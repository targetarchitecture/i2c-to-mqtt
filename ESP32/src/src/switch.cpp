#include <Arduino.h>
#include "switch.h"
#include <SparkFunSX1509.h> // Include SX1509 library

SX1509 switches; // Create an SX1509 object to be used throughout

TaskHandle_t SwitchTask;
byte pinState[15] = {HIGH};

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
    switches.clock(INTERNAL_CLOCK_2MHZ);
    switches.debounceTime(32); //64

    for (size_t i = 0; i < 16; i++)
    {
        switches.pinMode(i, INPUT_PULLUP);
        switches.debouncePin(i);

        //Serial.print("debouncePin:");
        //Serial.println(i);
    }

    //give back the i2c flag for the next task
    xSemaphoreGive(i2cSemaphore);

    xTaskCreate(
        switch_task,   /* Task function. */
        "Switch Task", /* name of task. */
        2048 * 4,      /* Stack size of task (uxTaskGetStackHighWaterMark: 7728) */
        NULL,          /* parameter of the task */
        1,             /* priority of the task */
        &SwitchTask);  /* Task handle to keep track of created task */
}

void switch_task(void *pvParameters)
{
    //TODO: Ask Google if this is the best place to declare variables in an endless task
    byte newPinState[15] = {HIGH};

    for (size_t i = 0; i < 16; i++)
    {
        //set the pin states to HIGH as PULLUP is set
        pinState[i] = HIGH;
        newPinState[i] = HIGH;
    }

    //TODO: see if this improves the inital flood of readings
    delay(100);

    /* Inspect our own high water mark on entering the task. */
    UBaseType_t uxHighWaterMark;
    uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    Serial.print("switch_task uxTaskGetStackHighWaterMark:");
    Serial.println(uxHighWaterMark);

    for (;;)
    {
        //TODO: On SN4 there will be an wait for interupt here to prevent scanning if there's no event occured

        // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        // Serial.print("switch_task uxTaskGetStackHighWaterMark:");
        // Serial.println(uxHighWaterMark);

        //quickly read all of the pins and save the state

        //wait for the i2c semaphore flag to become available
        xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

        for (size_t i = 0; i < 16; i++)
        {
            newPinState[i] = switches.digitalRead(i);
        }

        //give back the i2c flag for the next task
        xSemaphoreGive(i2cSemaphore);

        for (size_t i = 0; i < 16; i++)
        {
            //check for differance
            if (pinState[i] != newPinState[i])
            {
                Serial.printf("Readings (%d): %d,%d \n", i, pinState[i], newPinState[i]);

                char msgtosend[MAXBBCMESSAGELENGTH];
                sprintf(msgtosend, "E%d,%d", i + 1, newPinState[i]);

                Serial.print("msgtosend:");
                Serial.println(msgtosend);

                sendToMicrobit(msgtosend);

                pinState[i] = newPinState[i];
            }
        }

        //TODO: On SN4 change this to wait for interupt
        // put a delay so it isn't overwhelming
        delay(50);
    }

    vTaskDelete(NULL);
}
