#include <Arduino.h>
#include "ADC.h"

TaskHandle_t ADCTask;

volatile uint32_t ADCPollingRate = 500;

volatile bool ADC1Enabled = false;
volatile bool ADC2Enabled = false;

#define ADCWINDOW_SIZE 5

long ADC1INDEX = 0;
long ADC1VALUE = 0;
long ADC1SUM = 0;
long ADC1READINGS[ADCWINDOW_SIZE];
long ADC1AVERAGED = 0;

long ADC2INDEX = 0;
long ADC2VALUE = 0;
long ADC2SUM = 0;
long ADC2READINGS[ADCWINDOW_SIZE];
long ADC2AVERAGED = 0;


void ADC_setup()
{
    pinMode(ADC1, INPUT);
    pinMode(ADC2, INPUT);

    xTaskCreatePinnedToCore(
        ADC_task,          /* Task function. */
        "ADC Task",        /* name of task. */
        3500,              /* Stack size of task (uxTaskGetStackHighWaterMark = 3208)*/
        NULL,              /* parameter of the task */
        ADC_task_Priority, /* priority of the task */
        &ADCTask, 1);      /* Task handle to keep track of created task */
}

long ReadADC2()
{
    ADC2SUM = ADC2SUM - ADC2READINGS[ADC2INDEX];        // Remove the oldest entry from the sum
    ADC2VALUE = map(analogRead(ADC2), 0, 4095, 0, 100); // Read the next sensor value
    ADC2READINGS[ADC2INDEX] = ADC2VALUE;                // Add the newest reading to the window
    ADC2SUM = ADC2SUM + ADC2VALUE;                      // Add the newest reading to the sum
    ADC2INDEX = (ADC2INDEX + 1) % ADCWINDOW_SIZE;       // Increment the index, and wrap to 0 if it exceeds the window size

    ADC2AVERAGED = ADC2SUM / ADCWINDOW_SIZE; // Divide the sum of the window by the window size for the result

    return ADC2AVERAGED;
}

long ReadADC1()
{
    ADC1SUM = ADC1SUM - ADC1READINGS[ADC1INDEX];        // Remove the oldest entry from the sum
    ADC1VALUE = map(analogRead(ADC1), 0, 4095, 0, 100); // Read the next sensor value
    ADC1READINGS[ADC1INDEX] = ADC1VALUE;                // Add the newest reading to the window
    ADC1SUM = ADC1SUM + ADC1VALUE;                      // Add the newest reading to the sum
    ADC1INDEX = (ADC1INDEX + 1) % ADCWINDOW_SIZE;       // Increment the index, and wrap to 0 if it exceeds the window size

    ADC1AVERAGED = ADC1SUM / ADCWINDOW_SIZE; // Divide the sum of the window by the window size for the result

    return ADC1AVERAGED;
}

void ADC_task(void *pvParameters)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("ADC_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    //loop around quickly to populate the arrays
    for (size_t i = 0; i < 50; i++)
    {
        ReadADC1();
        ReadADC2();

        delay(10);
    }

    for (;;)
    {
        if (ADC1Enabled == true)
        {
            long ADC1_VALUE =  ReadADC1();

            char msg[MAXBBCMESSAGELENGTH] = {0};
            sprintf(msg, "C1,%d", ADC1_VALUE);

            sendToMicrobit(msg);
        }

        if (ADC2Enabled == true)
        {
            long ADC2_VALUE =  ReadADC2();

            char msg[MAXBBCMESSAGELENGTH] = {0};
            sprintf(msg, "C2,%d", ADC2_VALUE);

            //Serial.println(msg);

            sendToMicrobit(msg);
        }

        //change how this works to if enabled each ADC will send it's value every 1/2 second
        delay(ADCPollingRate);
    }

    vTaskDelete(NULL);
}

// void ADC_task_OLD(void *pvParameters)
// {
//     // UBaseType_t uxHighWaterMark;
//     // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
//     // Serial.print("ADC_task uxTaskGetStackHighWaterMark:");
//     // Serial.println(uxHighWaterMark);

//     long newADC1value = 10000;
//     long newADC2value = 10000;

//     for (;;)
//     {
//         if (ADC1Enabled == true)
//         {
//             newADC1value = map(analogRead(ADC1), 0, 4095, 0, 100);

//             if (newADC1value != ADC1_VALUE)
//             {
//                 char msg[MAXBBCMESSAGELENGTH] = {0};
//                 sprintf(msg, "C1,%d", newADC1value);

//                 sendToMicrobit(msg);

//                 ADC1_VALUE = newADC1value;
//             }
//         }

//         if (ADC2Enabled == true)
//         {
//             newADC2value = map(analogRead(ADC2), 0, 4095, 0, 100);

//             if (newADC2value != ADC2_VALUE)
//             {
//                 char msg[MAXBBCMESSAGELENGTH] = {0};
//                 sprintf(msg, "C2,%d", newADC2value);

//                 //Serial.println(msg);

//                 sendToMicrobit(msg);

//                 ADC2_VALUE = newADC2value;
//             }
//         }

//         if (ADC1Enabled == false && ADC2Enabled == false)
//         {
//             //take your time to do nothing.
//             delay(1000);
//         }
//         else
//         {
//             delay(500);
//         }
//     }

//     vTaskDelete(NULL);
// }

void ADC_deal_with_message(char msg[MAXESP32MESSAGELENGTH])
{
    if (strncmp(msg, "U1,0", 4) == 0)
    {
        //Serial.println("ADC1Enabled = false");
        ADC1Enabled = false;
    }
    else if (strncmp(msg, "U1,1", 4) == 0)
    {
        //Serial.println("ADC1Enabled = true");
        ADC1Enabled = true;
    }
    else if (strncmp(msg, "U2,0", 4) == 0)
    {
        //Serial.println("ADC2Enabled = false");
        ADC2Enabled = false;
    }
    else if (strncmp(msg, "U2,1", 4) == 0)
    {
        ADC2Enabled = true;
        //Serial.println("ADC2Enabled = true");
    }

    else if (strncmp(msg, "U3", 2) == 0)
    {
        //TODO: see if need this copy of msg
        std::string X = msg;

        messageParts parts = processQueueMessage(X.c_str(), "ADC");

        ADCPollingRate = atoi(parts.value1);
    }
}