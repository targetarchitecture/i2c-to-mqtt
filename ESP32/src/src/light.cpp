#include <Arduino.h>
#include "light.h"
#include <SparkFunSX1509.h> // Include SX1509 library
#include <iostream>
#include <string>
#include <vector>

SX1509 lights; // Create an SX1509 object to be used throughout

enum pinState
{
    on,
    off,
    blink,
    breathe
};
//pinState pinStates[16] = {off};
std::vector<pinState> pinStates;

TaskHandle_t LightTask;

void light_setup()
{
    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    if (!lights.begin(0x3E))
    {
        Serial.println("SX1509 for lighting not found");

        POST(4);
    }

    // Use the internal 2MHz oscillator.
    //lights.clock(INTERNAL_CLOCK_2MHZ, 4);

    checkI2Cerrors("light");

    //give back the i2c flag for the next task
    xSemaphoreGive(i2cSemaphore);

    xTaskCreatePinnedToCore(
        light_task,          /* Task function. */
        "Light Task",        /* name of task. */
        2048 * 4,            /* Stack size of task (uxTaskGetStackHighWaterMark:??) */
        NULL,                /* parameter of the task */
        light_task_Priority, /* priority of the task */
        &LightTask, 1);      /* Task handle to keep track of created task */
}

void light_task(void *pvParameters)
{
    //TODO: Ask Google if this is the best place to declare variables in an endless task
    messageParts parts;
    // int16_t currentVolume;
    // int16_t currentTrack;

    for (int pin = 0; pin < 16; pin++)
    {
        pinStates.push_back(off);
    }

    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("light_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    // Serial.printf("Light task is on core %i\n", xPortGetCoreID());

    for (;;)
    {
        char msg[MAXESP32MESSAGELENGTH] = {0};

        //uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        //Serial.print("light_task uxTaskGetStackHighWaterMark:");
        //Serial.println(uxHighWaterMark);

        //wait for new music command in the queue
        xQueueReceive(Light_Queue, &msg, portMAX_DELAY);

        //Serial.print("Light_Queue:");
        //Serial.println(msg);

        //TODO: see if need this copy of msg
        std::string X = msg;

        parts = processQueueMessage(X.c_str(), "LIGHT");

        // Serial.print("action:");
        // Serial.print(parts.identifier);
        // Serial.print(" @ ");
        // Serial.println(millis());

        /*
        The breathable pins are 4, 5, 6, 7, 12, 13, 14, 15 only. If tRise and
        tFall are set on 0-3 or 8-11 those pins will still only blink.
        ledDriverInit should be called on the pin to be blinked before this.
        */

        if (strcmp(parts.identifier, "Y1") == 0)
        {
            byte pin = constrain(atoi(parts.value1), 0, 15);
            long tOn = atol(parts.value2);
            long tOff = atol(parts.value3);

            //stop the LED first
            stopLight(pin);

            //wait for the i2c semaphore flag to become available
            xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

            lights.pinMode(pin, OUTPUT);  // Set LED pin to OUTPUT
            lights.blink(pin, tOn, tOff); // Blink the LED pin -- ~1000 ms LOW, ~500 ms HIGH:

            checkI2Cerrors("light Y1");

            //give back the i2c flag for the next task
            xSemaphoreGive(i2cSemaphore);

            //set method for the pins so we can figure out how to turn it off
            pinStates[pin] = blink;
        }
        else if (strcmp(parts.identifier, "Y2") == 0)
        {
            byte pin = constrain(atoi(parts.value1), 0, 15);
            long tOn = atol(parts.value2);
            long tOff = atol(parts.value3);
            long rise = atol(parts.value4);
            long fall = atol(parts.value5);

            //stop the LED first
            stopLight(pin);

            //wait for the i2c semaphore flag to become available
            xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

            lights.pinMode(pin, ANALOG_OUTPUT);         // To breathe an LED, make sure you set it as an ANALOG_OUTPUT, so we can PWM the pin
            lights.breathe(pin, tOn, tOff, rise, fall); // Breathe an LED: 1000ms LOW, 500ms HIGH, 500ms to rise from low to high, 250ms to fall from high to low

            checkI2Cerrors("light Y2");

            //give back the i2c flag for the next task
            xSemaphoreGive(i2cSemaphore);

            //set method for the pins so we can figure out how to turn it off
            pinStates[pin] = breathe;
        }
        else if (strcmp(parts.identifier, "Y3") == 0)
        {
            byte pin = constrain(atoi(parts.value1), 0, 15);
            int tOnOff = atoi(parts.value2);

            //stop the LED first
            stopLight(pin);

            //see what state we need
            if (tOnOff == 1)
            {
                //set method for the pins so we can figure out how to turn it off
                pinStates[pin] = on;

                //wait for the i2c semaphore flag to become available
                xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

                lights.pinMode(pin, OUTPUT);    // Set LED pin to OUTPUT
                lights.digitalWrite(pin, HIGH); //set to ON for Ada!

                checkI2Cerrors("light Y3");

                //give back the i2c flag for the next task
                xSemaphoreGive(i2cSemaphore);
            }
        }
        else if (strcmp(parts.identifier, "Y4") == 0)
        {
            //Serial.println("YOYOYO");

            //wait for the i2c semaphore flag to become available
            xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

            lights.reset(0);

            // Set the clock to a default of 2MHz using internal
            lights.clock(INTERNAL_CLOCK_2MHZ);

            checkI2Cerrors("light Y4");

            //give back the i2c flag for the next task
            xSemaphoreGive(i2cSemaphore);
        }
    }

    vTaskDelete(NULL);
}

void stopLight(long pin)
{
    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    if (pinStates[pin] == on)
    {
        lights.digitalWrite(pin, LOW); //set to OFF
    }
    if (pinStates[pin] == blink)
    {
        //io.pinMode(SX1509_LED_PIN, OUTPUT);
        //io.digitalWrite(SX1509_LED_PIN, LOW);

        lights.setupBlink(pin, 0, 0);
    }
    if (pinStates[pin] == breathe)
    {
        lights.setupBlink(pin, 0, 0);
    }

    checkI2Cerrors("light (stoplight)");

    //give back the i2c flag for the next task
    xSemaphoreGive(i2cSemaphore);

    //set method for the pins so we can figure out how to turn it
    pinStates[pin] = off;
}