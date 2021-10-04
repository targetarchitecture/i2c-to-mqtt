#include <Arduino.h>
#include "DAC.h"

TaskHandle_t DACTask;
//QueueHandle_t DAC_Queue;

void DAC_setup()
{
    //DAC_Queue = xQueueCreate(50, sizeof(RXfromBBCmessage));

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

    messageParts parts;
    uint8_t DACvalue;

    for (;;)
    {
        char msg[MAXESP32MESSAGELENGTH] = {0};

        //wait for new DAC commands in the queue
        xQueueReceive(DAC_Queue, &msg, portMAX_DELAY);

        Serial << "DAC_Queue: " << msg << endl;

        //TODO: see if need this copy of msg
        std::string X = msg;

        messageParts parts = processQueueMessage(X, "DAC");

        DACvalue = std::stoi(parts.value1);

        //  constrain(std::stoi(parts.value1), 0, 254);

        // Serial << "DAC action: " << parts.identifier << " : " << DACvalue << " : " << parts.value1 << endl;

        DACvalue = constrain(DACvalue, 0, 254);

        //   Serial << "DAC action 2: " << parts.identifier << " : " << DACvalue << " : " << parts.value1 << endl;

        if (strncmp(parts.identifier, "DIAL1", 5) == 0)
        {
            dacWrite(DAC1, DACvalue);

            Serial << "DAC1: " << DACvalue<< endl;
        }
        else if (strncmp(parts.identifier, "DIAL2", 5) == 0)
        {
            dacWrite(DAC2, DACvalue);

             Serial << "DAC2: " << DACvalue<< endl;
        }
    }

    vTaskDelete(NULL);
}