#include <Arduino.h>
#include "encoders.h"
#include <ESP32Encoder.h>

TaskHandle_t EncodersTask;

ESP32Encoder encoder1;
ESP32Encoder encoder2;

volatile int32_t encoder1Count;
volatile int32_t encoder2Count;

volatile bool encoder1Enabled = false;
volatile bool encoder2Enabled = false;

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
    xTaskCreate(&encoders_task, "encoders_task", 2048, NULL, 1, &EncodersTask);
}

void encoders_deal_with_message(char msg[MAXMESSAGELENGTH])
{
    if (strcmp(msg, "W1,0") == 0)
    {
        encoder1Enabled = false;
    }
    else if (strcmp(msg, "W1,1") == 0)
    {
        encoder1Enabled = true;
    }
    else if (strcmp(msg, "W2,0") == 0)
    {
        encoder2Enabled = false;
    }
    else if (strcmp(msg, "W2,1") == 0)
    {
        encoder2Enabled = true;
    }
}

void encoders_task(void *pvParameter)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("encoders_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    int32_t newEncoder1Count = 0;
    int32_t newEncoder2Count = 0;

    for (;;)
    {
        if (encoder1Enabled == true)
        {
            newEncoder1Count = encoder1.getCount();

            if (newEncoder1Count != encoder1Count)
            {
                if (newEncoder1Count > encoder1Count)
                {
                    sendToMicrobit("D1,+");
                }
                else
                {
                    sendToMicrobit("D1,-");
                }

                encoder1Count = newEncoder1Count;
            }
        }

        if (encoder2Enabled == true)
        {
            newEncoder2Count = encoder2.getCount();

            if (newEncoder2Count != encoder2Count)
            {
                if (newEncoder2Count > encoder2Count)
                {
                    sendToMicrobit("D2,+");
                }
                else
                {
                    sendToMicrobit("D2,-");
                }

                encoder2Count = newEncoder2Count;
            }
        }

        if (encoder1Enabled == false && encoder2Enabled == false)
        {
            //take your time to do nothing
            delay(1000);
        }
        else
        {
            delay(500);
        }
    }

    vTaskDelete(NULL);
}