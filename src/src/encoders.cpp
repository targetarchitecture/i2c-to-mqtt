#include <Arduino.h>
#include "encoders.h"
#include <ESP32Encoder.h>

TaskHandle_t EncodersTask;

ESP32Encoder encoder1;
ESP32Encoder encoder2;

volatile int32_t encoder1Count=0;
volatile int32_t encoder2Count=0;

void encoders_setup()
{
    // Enable the weak pull up/down resistors
    ESP32Encoder::useInternalWeakPullResistors = DOWN; // UP;

    // Attach pins for use as encoder pins
    encoder1.attachHalfQuad(ROTARY1CK, ROTARY1DT);
    encoder1.clearCount();
    encoder1.setCount(0);

    // Attach pins for use as encoder pins
    encoder2.attachHalfQuad(ROTARY2CK, ROTARY2DT);
    encoder2.clearCount();
    encoder2.setCount(0);

    //start task to read an send rotary information every 500ms (uxTaskGetStackHighWaterMark = 1560)
    xTaskCreatePinnedToCore(
        &encoders_task,
        "encoders_task",
        2048,
        NULL,
        encoders_task_Priority,
        &EncodersTask,
        1);
}

void encoders_task(void *pvParameter)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("encoders_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    for (;;)
    {
        encoder1Count = encoder1.getCount();
        encoder2Count = encoder2.getCount();

        delay(500);
    }

    vTaskDelete(NULL);
}