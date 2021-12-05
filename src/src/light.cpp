#include <Arduino.h>
#include "light.h"

SX1509 lights; // Create an SX1509 object to be used throughout

std::vector<LED> LEDs;

TaskHandle_t LightTask;

extern SemaphoreHandle_t i2cSemaphore;

void light_setup()
{

    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    if (!lights.begin(0x3E))
    {
        Serial.println("SX1509 for lighting not found");

        POST(4);
    }

    checkI2Cerrors("light");

    //give back the i2c flag for the next task
    xSemaphoreGive(i2cSemaphore);

    //setup the lights to be off
    for (int pin = 0; pin < 16; pin++)
    {
        LED l;

        l.pin = pin;
        l.state = pinState::off;
        l.taskHandle = NULL; //create the task handles

        LEDs.push_back(l);

        //wait for the i2c semaphore flag to become available
        xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

        lights.pinMode(pin, ANALOG_OUTPUT); // Set LED pin to ANALOG as a default
        lights.ledDriverInit(pin);
        lights.analogWrite(pin, LEDOFF);

        checkI2Cerrors("light");

        //give back the i2c flag for the next task
        xSemaphoreGive(i2cSemaphore);
    }

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

    for (;;)
    {
        messageParts parts;

        //wait for new music command in the queue
        xQueueReceive(Light_Queue, &parts, portMAX_DELAY);

        std::string identifier = parts.identifier;

        //Serial << identifier.c_str() << endl;

        //put a delay in to let the queue catch up
        delay(10);

        if (identifier.compare("LBLINK") == 0)
        {
            //stop any previous tasks
            stopCurrentTaskOnPin(parts.value1);

            LEDs[parts.value1].OnTimeMS = parts.value2;
            LEDs[parts.value1].OffTimeMS = parts.value3;

            //set method for the pins so we can figure out how to turn it off
            LEDs[parts.value1].state = blink;

            //Serial << identifier.c_str() << "," << parts.value1 << "," << parts.value2 << "," << parts.value3 << endl;

            const char *taskName = "LED Task " + parts.value1;

            xTaskCreatePinnedToCore(
                &LEDBlinkingTask,
                taskName,
                4000,
                NULL,
                LEDBlinkingTask_Priority,
                &LEDs[parts.value1].taskHandle,
                1);

            xTaskNotify(LEDs[parts.value1].taskHandle, parts.value1, eSetValueWithOverwrite);
        }
        else if (identifier.compare("LBREATHE") == 0)
        {
            //stop any previous tasks
            stopCurrentTaskOnPin(parts.value1);

            LEDs[parts.value1].OnTimeMS = parts.value2;
            LEDs[parts.value1].OffTimeMS = parts.value3;
            LEDs[parts.value1].RiseTimeMS = parts.value4;
            LEDs[parts.value1].FallTimeMS = parts.value5;

            //set method for the pins so we can figure out how to turn it off
            LEDs[parts.value1].state = breathe;

            const char *taskName = "LED Task " + parts.value1;

            xTaskCreatePinnedToCore(
                &LEDBreathingTask,
                taskName,
                4000,
                NULL,
                LEDBreathingTask_Priority,
                &LEDs[parts.value1].taskHandle,
                1);

            xTaskNotify(LEDs[parts.value1].taskHandle, parts.value1, eSetValueWithOverwrite);
        }
        else if (identifier.compare("LLEDONOFF") == 0)
        {
            TurnLEDOnOff(parts.value1, parts.value2);
        }
        else if (identifier.compare("LLEDALLOFF") == 0)
        {
            //turn on all LEDs - using the queue
            for (uint32_t i = 0; i <= 15; i++)
            {
                TurnLEDOnOff(i, 0);
            }
        }
        else if (identifier.compare("LLEDALLON") == 0)
        {
            //turn on all LEDs - using the queue
            for (uint32_t i = 0; i <= 15; i++)
            {
                TurnLEDOnOff(i, 1);
            }
        }
    }

    vTaskDelete(NULL);
}

void stopCurrentTaskOnPin(uint32_t pin)
{ //stop any previous tasks
    if (LEDs[pin].taskHandle != NULL)
    {
        vTaskDelete(LEDs[pin].taskHandle);
        delay(1);
        LEDs[pin].taskHandle = NULL;
    }
}

void TurnLEDOnOff(uint32_t pin, int OnOff)
{
    //stop any previous tasks
    stopCurrentTaskOnPin(pin);

    //see what state we need
    if (OnOff == 1)
    {
        //set method for the pins so we can figure out how to turn it off
        LEDs[pin].state = on;

        //wait for the i2c semaphore flag to become available
        xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

        lights.analogWrite(pin, LEDON); //set to ON for Ada!

        checkI2Cerrors("light on/off");

        //give back the i2c flag for the next task
        xSemaphoreGive(i2cSemaphore);
    }
    else
    {
        //set method for the pins so we can figure out how to turn it off
        LEDs[pin].state = off;

        //wait for the i2c semaphore flag to become available
        xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

        lights.analogWrite(pin, LEDOFF); //set to ON for Ada!

        checkI2Cerrors("light on/off");

        //give back the i2c flag for the next task
        xSemaphoreGive(i2cSemaphore);
    }
}

void LEDBlinkingTask(void *pvParameter)
{
    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("LEDBlinkingTask uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    uint32_t pin = 0;
    BaseType_t xResult = xTaskNotifyWait(0X00, 0x00, &pin, portMAX_DELAY);

    // Serial.print("LED blinking task for pin: ");
    // Serial.print(pin);
    // Serial.print(" on core ");
    // Serial.println(xPortGetCoreID());
    // Serial.print("Blink rate ");
    // Serial.print(LEDs[pin].OnTimeMS);
    // Serial.print(" / ");
    // Serial.println(LEDs[pin].OffTimeMS);

    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    lights.analogWrite(pin, LEDOFF); //always turn off the light

    checkI2Cerrors("light blinking");

    //give back the i2c flag for the next task
    xSemaphoreGive(i2cSemaphore);

    for (;;)
    {
        //wait for the i2c semaphore flag to become available
        xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

        lights.analogWrite(pin, LEDON); //set to ON

        //give back the i2c flag for the next task
        xSemaphoreGive(i2cSemaphore);

        delay(LEDs[pin].OnTimeMS);

        //wait for the i2c semaphore flag to become available
        xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

        lights.analogWrite(pin, LEDOFF); //set to OFF

        //give back the i2c flag for the next task
        xSemaphoreGive(i2cSemaphore);

        delay(LEDs[pin].OffTimeMS);
    }

    /* The task is going to be deleted. Set the handle to NULL. */
    LEDs[pin].taskHandle = NULL;

    //delete task
    vTaskDelete(NULL);
}

void LEDBreathingTask(void *pvParameter)
{
    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("LEDBreathingTask uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    uint32_t pin = 0;
    BaseType_t xResult = xTaskNotifyWait(0X00, 0x00, &pin, portMAX_DELAY);

    // Serial.print("LED breathing task for pin: ");
    // Serial.print(pin);
    // Serial.print(" on core ");
    // Serial.println(xPortGetCoreID());
    // Serial.print("Blink rate ");
    // Serial.print(LEDs[pin].OnTimeMS);
    // Serial.print(" / ");
    // Serial.println(LEDs[pin].OffTimeMS);

    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    lights.analogWrite(pin, LEDOFF); //always turn off the light

    checkI2Cerrors("light breathing");

    //give back the i2c flag for the next task
    xSemaphoreGive(i2cSemaphore);

    for (;;)
    {
        for (int i = 255; i >= 0; i--)
        {
            //wait for the i2c semaphore flag to become available
            xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

            // PWM the LED from 255 to 0
            lights.analogWrite(pin, i);

            //give back the i2c flag for the next task
            xSemaphoreGive(i2cSemaphore);

            delay(LEDs[pin].RiseTimeMS / 255); // Delay between each PWM
        }

        delay(LEDs[pin].OnTimeMS);

        for (int i = 0; i <= 255; i++)
        {
            //wait for the i2c semaphore flag to become available
            xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

            // PWM the LED from 0 to 255
            lights.analogWrite(pin, i);

            //give back the i2c flag for the next task
            xSemaphoreGive(i2cSemaphore);

            delay(LEDs[pin].FallTimeMS / 255); // Delay between each PWM
        }

        delay(LEDs[pin].OffTimeMS);
    }

    /* The task is going to be deleted. Set the handle to NULL. */
    LEDs[pin].taskHandle = NULL;

    //delete task
    vTaskDelete(NULL);
}