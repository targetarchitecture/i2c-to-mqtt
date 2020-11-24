#include <Arduino.h>
#include "ADC.h"

TaskHandle_t ADCTask;

volatile long ADC1_VALUE;
volatile long ADC2_VALUE;

// volatile bool ADC1Enabled = false;
// volatile bool ADC2Enabled = false;

void ADC_setup()
{
    pinMode(ADC1, INPUT);
    pinMode(ADC2, INPUT);

    ADC1_VALUE = map(analogRead(ADC1), 0, 4095, 0, 100);
    ADC2_VALUE = map(analogRead(ADC2), 0, 4095, 0, 100);

    xTaskCreate(
        ADC_task,   /* Task function. */
        "ADC Task", /* name of task. */
        3500,       /* Stack size of task (uxTaskGetStackHighWaterMark = 3208)*/
        NULL,       /* parameter of the task */
        1,          /* priority of the task */
        &ADCTask);  /* Task handle to keep track of created task */
}

void ADC_task(void *pvParameters)
{
    UBaseType_t uxHighWaterMark;
    uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    Serial.print("ADC_task uxTaskGetStackHighWaterMark:");
    Serial.println(uxHighWaterMark);

    long newADC1value;
    long newADC2value;

    for (;;)
    {
        //WORTH A PUNT TO SEE IF THE ROGUE READINGS ARE DUE TO THIS
        //wait for the i2c semaphore flag to become available
        xSemaphoreTake(i2cSemaphore, portMAX_DELAY);

        newADC1value = map(analogRead(ADC1), 0, 4095, 0, 100);
        newADC2value = map(analogRead(ADC2), 0, 4095, 0, 100);

        //give back the i2c flag for the next task
        xSemaphoreGive(i2cSemaphore);

        if (newADC1value != ADC1_VALUE)
        {
            // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            // Serial.print("ADC_task uxTaskGetStackHighWaterMark:");
            // Serial.println(uxHighWaterMark);

            char msg[MAXBBCMESSAGELENGTH] = {0};
            sprintf(msg, "C1,%d", newADC1value);

            sendToMicrobit(msg);

            ADC1_VALUE = newADC1value;
        }

        if (newADC2value != ADC2_VALUE)
        {
            // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            // Serial.print("ADC_task uxTaskGetStackHighWaterMark:");
            // Serial.println(uxHighWaterMark);

            char msg[MAXBBCMESSAGELENGTH] = {0};
            sprintf(msg, "C2,%d", newADC2value);

            sendToMicrobit(msg);

            ADC2_VALUE = newADC2value;
        }

        delay(50);
    }

    vTaskDelete(NULL);
}