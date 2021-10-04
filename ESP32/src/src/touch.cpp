#include <Arduino.h>
#include "touch.h"
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
volatile uint16_t lasttouched = 0;
volatile uint16_t currtouched = 0;

TaskHandle_t TouchTask;

volatile uint8_t debounceDelay = 0; // the debounce time; increase if the output flickers

std::string touchStates = "XXXXXXXXXXXX";

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
    pinMode(TOUCH_INT, INPUT_PULLUP);
    attachInterrupt(TOUCH_INT, handleTouchInterupt, FALLING);

    xTaskCreatePinnedToCore(
        &touch_task,
        "Touch Task",
        2000, //uxTaskGetStackHighWaterMark = 1750
        NULL,
        touch_task_Priority,
        &TouchTask,
        1);
}

void IRAM_ATTR handleTouchInterupt()
{
    xTaskNotify(TouchTask, 0, eSetValueWithoutOverwrite);
}

void touch_task(void *pvParameter)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("touch_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    uint32_t ulNotifiedValue = 0;
    BaseType_t xResult;

    unsigned long lastDebounceTime = 0; // the last time each output pin was toggled

    for (;;)
    {
        //SN7 there will be an wait for interupt here to prevent scanning if there's no event occured
        xResult = xTaskNotifyWait(0X00, 0x00, &ulNotifiedValue, portMAX_DELAY);

        //added a debouncing time delay
        if (millis() - lastDebounceTime >= debounceDelay)
        {
            lastDebounceTime = millis();

            delay(1);

            //wait for the i2c semaphore flag to become available
            xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

            checkI2Cerrors("Touch (start)");

            // Get the currently touched pads
            uint16_t currtouched = cap.touched();

            checkI2Cerrors("Touch (end)");

            //give back the i2c flag for the next task
            xSemaphoreGive(i2cSemaphore);

            //only bother sending if the touch changed
            if (currtouched != lasttouched)
            {
                for (uint8_t i = 0; i < 12; i++)
                {
                    // it if *is* touched and *wasnt* touched before, alert!
                    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)))
                    {
                        touchStates = touchStates.replace(i, 1, "H");
                    }

                    // if it *was* touched and now *isnt*, alert!
                    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)))
                    {
                        touchStates = touchStates.replace(i, 1, "L");
                    }
                }

                //remember last touch
                lasttouched = currtouched;
            }
        }
    }
    vTaskDelete(NULL);
}

void touch_deal_with_message(const char *msg)
{
    auto parts = processQueueMessage(msg, "TOUCH");

    if (strncmp(parts.identifier, "TTHRSLD", 7) == 0)
    {
        auto touchThreshold = std::stoi(parts.value1);
        auto releaseThreshold = std::stoi(parts.value2);

        cap.setThresholds(touchThreshold, releaseThreshold);
    }

    //overwrite bounce delay..
    if (strncmp(parts.identifier, "TBOUNCE", 7) == 0)
    {
        debounceDelay = std::stoi(parts.value1);
    }
}
