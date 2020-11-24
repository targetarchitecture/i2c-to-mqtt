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

    //uxTaskGetStackHighWaterMark = 1750
    xTaskCreate(&touch_task, "Touch Task", 2000, NULL, 1, &TouchTask);
}

void touch_task(void *pvParameter)
{
    UBaseType_t uxHighWaterMark;
    uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    Serial.print("touch_task uxTaskGetStackHighWaterMark:");
    Serial.println(uxHighWaterMark);

    //TODO: see if this delay prevents rogue first
    delay(100);

    for (;;)
    {
        //TODO: On SN4 there will be an wait for interupt here to prevent scanning if there's no event occured

        //wait for the i2c semaphore flag to become available
        xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

        // Get the currently touched pads
        currtouched = cap.touched();

        //give back the i2c flag for the next task
        xSemaphoreGive(i2cSemaphore);

        for (uint8_t i = 0; i < 12; i++)
        {
            // it if *is* touched and *wasnt* touched before, alert!
            if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)))
            {
                Serial.print(i);
                Serial.println(" touched");

                char msgtosend[MAXBBCMESSAGELENGTH];
                sprintf(msgtosend, "B1,%d", i);

                sendToMicrobit(msgtosend);

                // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
                // Serial.print("touch_task uxTaskGetStackHighWaterMark:");
                // Serial.println(uxHighWaterMark);
            }
            // if it *was* touched and now *isnt*, alert!
            if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)))
            {
                Serial.print(i);
                Serial.println(" released");

                char msgtosend[MAXBBCMESSAGELENGTH];
                sprintf(msgtosend, "B2,%d", i);

                sendToMicrobit(msgtosend);

                // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
                // Serial.print("touch_task uxTaskGetStackHighWaterMark:");
                // Serial.println(uxHighWaterMark);
            }
        }

        // reset our state
        lasttouched = currtouched;

        // put a delay so it isn't overwhelming
        delay(100);
    }

    vTaskDelete(NULL);
}