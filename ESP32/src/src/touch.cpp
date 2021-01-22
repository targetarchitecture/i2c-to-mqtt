#include <Arduino.h>
#include "touch.h"
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit))
#endif

Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

TaskHandle_t TouchTask;

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

    //uxTaskGetStackHighWaterMark = 1750
    xTaskCreatePinnedToCore(&touch_task, "Touch Task", 2000, NULL, 2, &TouchTask, 1);
}


void touch_task(void *pvParameter)
{
    uint32_t ulNotifiedValue = 0;
    BaseType_t xResult;

    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("touch_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    for (;;)
    {
        //SN7 there will be an wait for interupt here to prevent scanning if there's no event occured
        xResult = xTaskNotifyWait(0X00, 0x00, &ulNotifiedValue, portMAX_DELAY);

        delay(1);

        //wait for the i2c semaphore flag to become available
        xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

        checkI2Cerrors("Touch (start)");

        // Get the currently touched pads
        uint16_t currtouched = cap.touched();

        checkI2Cerrors("Touch (end)");

        //give back the i2c flag for the next task
        xSemaphoreGive(i2cSemaphore);

        for (uint8_t i = 0; i < 12; i++)
        {
            // it if *is* touched and *wasnt* touched before, alert!
            if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)))
            {
                // Serial.print(i);
                // Serial.println(" touched");

                char msgtosend[MAXBBCMESSAGELENGTH];
                sprintf(msgtosend, "B1,%d", i);

                sendToMicrobit(msgtosend);
            }

            // if it *was* touched and now *isnt*, alert!
            if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)))
            {
                // Serial.print(i);
                // Serial.println(" released");

                char msgtosend[MAXBBCMESSAGELENGTH];
                sprintf(msgtosend, "B2,%d", i);

                sendToMicrobit(msgtosend);
            }
        }

        // reset our state
        lasttouched = currtouched;
    }

    // debugging info, what
    // Serial.print("\t\t\t\t\t\t\t\t\t\t\t\t\t 0x");
    // Serial.println(cap.touched(), HEX);
    // Serial.print("Filt: ");
    // for (uint8_t i = 0; i < 12; i++)
    // {
    //     Serial.print(cap.filteredData(i));
    //     Serial.print("\t");
    // }
    // Serial.println();
    // Serial.print("Base: ");
    // for (uint8_t i = 0; i < 12; i++)
    // {
    //     Serial.print(cap.baselineData(i));
    //     Serial.print("\t");
    // }
    // Serial.println();
    // }

    vTaskDelete(NULL);
}

void IRAM_ATTR handleTouchInterupt()
{
    xTaskNotify(TouchTask, 0, eSetValueWithoutOverwrite);
}