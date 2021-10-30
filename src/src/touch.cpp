#include <Arduino.h>
#include "touch.h"
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched, so we know when buttons are 'released'
volatile uint16_t lasttouched = 0;
volatile uint16_t currtouched = 0;

TaskHandle_t TouchTask;

volatile uint8_t debounceDelay = 0; // the debounce time; increase if the output flickers

volatile int touchArray[12] = {};

void touch_setup()
{
    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    // Default address is 0x5A
    if (!cap.begin(0x5A))
    {
        Serial.println("MPR121 not found, check wiring?");

        POST(3);
    }

    //give back the i2c flag for the next task
    xSemaphoreGive(i2cSemaphore);

    //set-up the interupt
    //pinMode(TOUCH_INT, INPUT_PULLUP);
    //attachInterrupt(TOUCH_INT, handleTouchInterupt, FALLING);

    xTaskCreatePinnedToCore(
        &touch_task,
        "Touch Task",
        2000, //uxTaskGetStackHighWaterMark = 1750
        NULL,
        touch_task_Priority,
        &TouchTask,
        1);
}

// void IRAM_ATTR handleTouchInterupt()
// {
//     xTaskNotify(TouchTask, 0, eSetValueWithoutOverwrite);
// }

void touch_task(void *pvParameter)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("touch_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    //uint32_t ulNotifiedValue = 0;
    //BaseType_t xResult;

    //unsigned long lastDebounceTime = 0; // the last time each output pin was toggled

    //read once and set array as the baseline
    readAndSetArray();

    for (;;)
    {
        //SN7 there will be an wait for interupt here to prevent scanning if there's no event occured
        //xResult = xTaskNotifyWait(0X00, 0x00, &ulNotifiedValue, portMAX_DELAY);

        //added a debouncing time delay
        //if (millis() - lastDebounceTime >= debounceDelay)
        //{
        //lastDebounceTime = millis();

        delay(100);

        //read and set array returning the current touched
        auto currtouched = readAndSetArray();

        //only bother sending a touch update command if the touch changed
        if (currtouched != lasttouched)
        {
            std::string touchStates = "TUPDATE:";

            for (uint8_t i = 0; i < 12; i++)
            {
                if (touchArray[i] == 1)
                {
                    touchStates.append("H");
                }
                else
                {
                    touchStates.append("L");
                }
            }

            sendToMicrobit(touchStates);
        }

        //remember last touch
        lasttouched = currtouched;
    }
    vTaskDelete(NULL);
}

void touch_deal_with_message(messageParts message)
{
    std::string identifier = message.identifier;

    if (identifier.compare("TTHRSLD") == 0)
    {
        auto touchThreshold = message.value1;
        auto releaseThreshold = message.value2;

        cap.setThresholds(touchThreshold, releaseThreshold);
    }

    //overwrite bounce delay..
    if (identifier.compare("TBOUNCE") == 0)
    {
        debounceDelay = message.value1;
    }
}

uint16_t readAndSetArray()
{

    //wait for the i2c semaphore flag to become available
    xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

    checkI2Cerrors("Touch (start)");

    // Get the currently touched pads
    uint16_t currtouched = cap.touched();

    checkI2Cerrors("Touch (end)");

    //give back the i2c flag for the next task
    xSemaphoreGive(i2cSemaphore);

    //always update the array for the touch state command
    for (uint8_t i = 0; i < 12; i++)
    {
        // it if *is* touched and *wasnt* touched before, alert!
        if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)))
        {
            touchArray[i] = 1;
        }

        // if it *was* touched and now *isnt*, alert!
        if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)))
        {
            touchArray[i] = 0;
        }
    }

    return currtouched;
}