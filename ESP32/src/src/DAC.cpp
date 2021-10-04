#include <Arduino.h>
#include "DAC.h"

TaskHandle_t DACTask;

void DAC_setup()
{
    xTaskCreatePinnedToCore(
        DAC_task,          /* Task function. */
        "DAC Task",        /* name of task. */
        3500,              /* Stack size of task (uxTaskGetStackHighWaterMark = 3204) */
        NULL,              /* parameter of the task */
        DAC_task_Priority, /* priority of the task */
        &DACTask, 1);      /* Task handle to keep track of created task */
}

void DAC_task(void *pvParameters)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("DAC_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    uint8_t DACvalue;

    for (;;)
    {
        messageParts parts;

        //wait for new DAC commands in the queue
        xQueueReceive(DAC_Queue, &parts, portMAX_DELAY);

        std::string identifier = parts.identifier;

        //Serial << "DAC_Queue: " << identifier.c_str() << endl;

        DACvalue = constrain(parts.value1, 0, 254);

        if (identifier.compare("DIAL1") == 0)
        {
            dacWrite(DAC1, DACvalue);

            //Serial << "DAC1: " << DACvalue << endl;
        }
        else if (identifier.compare("DIAL2") == 0)
        {
            dacWrite(DAC2, DACvalue);

            //Serial << "DAC2: " << DACvalue << endl;
        }
    }

    vTaskDelete(NULL);
}