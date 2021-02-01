#include <Arduino.h>
#include "ADC.h"

TaskHandle_t ADCTask;
QueueHandle_t ADC_Queue;

volatile long ADC1_VALUE;
volatile long ADC2_VALUE;

volatile bool ADC1Enabled = false;
volatile bool ADC2Enabled = false;

void ADC_setup()
{
    pinMode(ADC1, INPUT);
    pinMode(ADC2, INPUT);

    ADC1_VALUE = map(analogRead(ADC1), 0, 4095, 0, 100);
    ADC2_VALUE = map(analogRead(ADC2), 0, 4095, 0, 100);

    xTaskCreatePinnedToCore(
        ADC_task,          /* Task function. */
        "ADC Task",        /* name of task. */
        3500,              /* Stack size of task (uxTaskGetStackHighWaterMark = 3208)*/
        NULL,              /* parameter of the task */
        ADC_task_Priority, /* priority of the task */
        &ADCTask, 1);      /* Task handle to keep track of created task */
}

void ADC_task(void *pvParameters)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("ADC_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    long newADC1value = 10000;
    long newADC2value = 10000;

    for (;;)
    {
        if (ADC1Enabled == true)
        {
            newADC1value = map(analogRead(ADC1), 0, 4095, 0, 100);

            if (newADC1value != ADC1_VALUE)
            {
                char msg[MAXBBCMESSAGELENGTH] = {0};
                sprintf(msg, "C1,%d", newADC1value);

                sendToMicrobit(msg);

                ADC1_VALUE = newADC1value;
            }
        }

        if (ADC2Enabled == true)
        {
            newADC2value = map(analogRead(ADC2), 0, 4095, 0, 100);

            if (newADC2value != ADC2_VALUE)
            {
                char msg[MAXBBCMESSAGELENGTH] = {0};
                sprintf(msg, "C2,%d", newADC2value);

                sendToMicrobit(msg);

                ADC2_VALUE = newADC2value;
            }
        }

        if (ADC1Enabled == false && ADC2Enabled == false)
        {
            //take your time to do nothing
            delay(1000);
        }
        else
        {
            delay(100);
        }
    }

    vTaskDelete(NULL);
}

void ADC_deal_with_message(char msg[MAXESP32MESSAGELENGTH])
{
    if (strcmp(msg, "U1,0") == 0)
    {
        ADC1Enabled = false;
    }
    else if (strcmp(msg, "U1,1") == 0)
    {
        ADC1Enabled = true;
    }
    else if (strcmp(msg, "U2,0") == 0)
    {
        ADC2Enabled = false;
    }
    else if (strcmp(msg, "U2,1") == 0)
    {
        ADC2Enabled = true;
    }
}