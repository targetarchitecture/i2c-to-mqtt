#include <Arduino.h>
#include "switch.h"
#include <SparkFunSX1509.h> // Include SX1509 library
#include <iostream>
#include <string>
#include <vector>

SX1509 switches; // Create an SX1509 object to be used throughout

TaskHandle_t SwitchTask;

std::vector<int> pinState;

void switch_setup()
{
    //create the vector
    for (size_t i = 0; i < 16; i++)
    {
        //set the pin states to HIGH as PULLUP is set
        pinState.push_back(0);
    }

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

        //set pin and read value
        if (switches.digitalRead(i) == LOW)
        {
            pinState[i] = 1;
        }
        else
        {
            pinState[i] = 0;
        }
    }

    checkI2Cerrors("switch");

    //give back the i2c flag for the next task
    xSemaphoreGive(i2cSemaphore);

    xTaskCreatePinnedToCore(
        switch_task,          /* Task function. */
        "Switch Task",        /* name of task. */
        8500,                 /* Stack size of task (uxTaskGetStackHighWaterMark: 7728) */
        NULL,                 /* parameter of the task */
        switch_task_Priority, /* priority of the task */
        &SwitchTask, 1);      /* Task handle to keep track of created task */

    //set-up the input pin
    pinMode(SWITCH_INT, INPUT_PULLUP);
}

void switch_task(void *pvParameters)
{
    /* Inspect our own high water mark on entering the task. */
    // BaseType_t xResult;
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("switch_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    std::vector<int> newPinState;

    for (size_t i = 0; i < 16; i++)
    {
        //set the pin states to HIGH as PULLUP is set
        newPinState.push_back(0); // HIGH;
    }

    for (;;)
    {
        //On SN7 there will be an wait for interupt here to prevent scanning if there's no event occured
        if (digitalRead(SWITCH_INT) == LOW)
        {
            //delay(1);

            // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            // Serial.print("switch_task uxTaskGetStackHighWaterMark:");
            // Serial.println(uxHighWaterMark);

            //quickly read all of the pins and save the state

            //wait for the i2c semaphore flag to become available
            xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

            checkI2Cerrors("switch (switch_task start)");

            for (size_t i = 0; i < 16; i++)
            {
                if (switches.digitalRead(i) == LOW)
                {
                    newPinState[i] = 1;
                }
                else //if (switches.digitalRead(i) == HIGH)
                {
                    newPinState[i] = 0;
                }
            }

            checkI2Cerrors("switch (switch_task end)");

            //give back the i2c flag for the next task
            xSemaphoreGive(i2cSemaphore);

            for (size_t i = 0; i < 16; i++)
            {
                //check for differance
                if (pinState[i] != newPinState[i])
                {
                    //Serial.printf("Readings (%i): %i,%i \n", i, pinState[i], newPinState[i]);

                    char msgtosend[MAXBBCMESSAGELENGTH];
                    sprintf(msgtosend, "E1,%i,%i", i, newPinState[i]);

                    sendToMicrobit(msgtosend);

                    // Serial.print("msgtosend:");
                    // Serial.println(msgtosend);

                    pinState[i] = newPinState[i];
                }
            }
        }

        // put a delay so it isn't overwhelming
        delay(50);
    }

    vTaskDelete(NULL);
}

void switch_deal_with_message(char msg[MAXESP32MESSAGELENGTH])
{
    auto parts = processQueueMessage(msg, "SWITCH");

    if (strncmp(parts.identifier, "R1",2) == 0)
    {
        read_and_send_pin_state();
    }
}

void read_and_send_pin_state()
{
    //wait for the i2c semaphore flag to become available.
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    checkI2Cerrors("switch (switch_deal_with_message)");

    for (size_t i = 0; i < 16; i++)
    {
        if (switches.digitalRead(i) == LOW)
        {
            pinState[i] = 1;
        }
        else
        {
            pinState[i] = 0;
        }
    }

    checkI2Cerrors("switch (switch_deal_with_message)");

    //give back the i2c flag for the next task
    xSemaphoreGive(i2cSemaphore);

    //build string to send back
    char msgtosend[MAXBBCMESSAGELENGTH];
    sprintf(msgtosend, "E2,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i,%i", pinState[0], pinState[1], pinState[2], pinState[3], pinState[4], pinState[5], pinState[6], pinState[7], pinState[8], pinState[9], pinState[10], pinState[11], pinState[12], pinState[13], pinState[14], pinState[15]);

    sendToMicrobit(msgtosend);

    // Serial.print("switch state :");
    // Serial.println(msgtosend);
}
