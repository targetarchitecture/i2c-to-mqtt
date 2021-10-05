#include <Arduino.h>
#include "light.h"
#include <SparkFunSX1509.h> // Include SX1509 library
#include <iostream>
#include <string>
#include <vector>

SX1509 lights; // Create an SX1509 object to be used throughout

//std::vector<pinState> pinStates;
std::vector<LED> LEDs;

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
    lights.clock(INTERNAL_CLOCK_2MHZ, 4);

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
    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("light_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    messageParts parts;

    for (int pin = 0; pin < 16; pin++)
    {
        LED l;

        l.pin = pin;
        l.state = pinState::off;
        l.taskHandle = NULL; //create the task handles

        LEDs.push_back(l);

        //pinStates.push_back(off);
    }

    for (;;)
    {
        messageParts parts;

        //wait for new music command in the queue
        xQueueReceive(Light_Queue, &parts, portMAX_DELAY);

        /*
        The breathable pins are 4, 5, 6, 7, 12, 13, 14, 15 only. If tRise and
        tFall are set on 0-3 or 8-11 those pins will still only blink.
        ledDriverInit should be called on the pin to be blinked before this.
        */

        std::string identifier = parts.identifier;

        if (identifier.compare(0, 6, "LBLINK") == 0)
        {
            byte pin = parts.value1;
            long tOn = parts.value2;
            long tOff = parts.value3;


            

            //stop the LED first
            stopLight(pin);

            //wait for the i2c semaphore flag to become available
            xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

            lights.pinMode(pin, OUTPUT);  // Set LED pin to OUTPUT
            lights.blink(pin, tOn, tOff); // Blink the LED pin -- ~1000 ms LOW, ~500 ms HIGH:

            checkI2Cerrors("light blinking");

            //give back the i2c flag for the next task
            xSemaphoreGive(i2cSemaphore);

            //set method for the pins so we can figure out how to turn it off
            pinStates[pin] = blink;
        }
        else if (identifier.compare("LBREATHE") == 0)
        {
            byte pin = constrain(parts.value1, 0, 15);
            long tOn = parts.value2;
            long tOff = parts.value3;
            long rise = parts.value4;
            long fall = parts.value5;

            //stop the LED first
            stopLight(pin);

            //wait for the i2c semaphore flag to become available
            xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

            lights.pinMode(pin, ANALOG_OUTPUT);         // To breathe an LED, make sure you set it as an ANALOG_OUTPUT, so we can PWM the pin
            lights.breathe(pin, tOn, tOff, rise, fall); // Breathe an LED: 1000ms LOW, 500ms HIGH, 500ms to rise from low to high, 250ms to fall from high to low

            checkI2Cerrors("light breathing");

            //give back the i2c flag for the next task
            xSemaphoreGive(i2cSemaphore);

            //set method for the pins so we can figure out how to turn it off
            pinStates[pin] = breathe;
        }
        else if (identifier.compare("LLEDONOFF") == 0)
        {
            byte pin = parts.value1;
            int tOnOff = parts.value2;

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

                checkI2Cerrors("light on/off");

                //give back the i2c flag for the next task
                xSemaphoreGive(i2cSemaphore);
            }
        }
        else if (identifier.compare("LLEDALLOFF") == 0)
        {
            //wait for the i2c semaphore flag to become available
            //xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

            //lights.reset(1);

            //turn on all LEDs - using the queue
            for (int i = 0; i <= 15; i++)
            {
                messageParts msgtosend = {};

                strcpy(msgtosend.identifier, "LLEDONOFF");
                msgtosend.value1 = i;
                msgtosend.value2 = 0;

                xQueueSend(Light_Queue, &msgtosend, portMAX_DELAY);
            }

            // Set the clock to a default of 2MHz using internal
            //lights.clock(INTERNAL_CLOCK_2MHZ);

            //checkI2Cerrors("light Y4");

            //give back the i2c flag for the next task
            //xSemaphoreGive(i2cSemaphore);
        }
        else if (identifier.compare("LLEDALLON") == 0)
        {
            //turn on all LEDs - using the queue
            for (int i = 0; i <= 15; i++)
            {
                messageParts msgtosend = {};

                strcpy(msgtosend.identifier, "LLEDONOFF");
                msgtosend.value1 = i;
                msgtosend.value2 = 1;

                xQueueSend(Light_Queue, &msgtosend, portMAX_DELAY);
            }
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