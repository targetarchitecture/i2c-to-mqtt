#include <Arduino.h>
#include "microbit.h"

TaskHandle_t MicrobitRXTask;
TaskHandle_t MicrobitTXTask;

void microbit_setup()
{
    /* Configure parameters of an UART driver, communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(BBC_UART_NUM, &uart_config);

    //Set UART pins to the BBC Microbit (UART2)
    uart_set_pin(BBC_UART_NUM, MICROBIT_RX, MICROBIT_TX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    //Install UART driver, and get the queue (buffer size = 2048)
    uart_driver_install(BBC_UART_NUM, UARTMESSAGELENGTH * 8 * 2, UARTMESSAGELENGTH * 8 * 2, 50, &Microbit_Receive_Queue, 0);

    //Create a task to handler UART event from ISR
    xTaskCreatePinnedToCore(microbit_receive_task, "Microbit RX Task", 2048, NULL, 12, &MicrobitRXTask,1);

    //Create a task to handle send UART data to Microbit
    xTaskCreatePinnedToCore(microbit_transmit_task, "Microbit TX Task", 2048, NULL, 1, &MicrobitTXTask,1);
}

void microbit_receive_task(void *pvParameters)
{
//Serial.printf("Microbit RX task is on core %i\n", xPortGetCoreID());

    uart_event_t event;
    uint8_t *dtmp = (uint8_t *)malloc(UARTMESSAGELENGTH * 8);

    for (;;)
    {
        //Waiting for UART event.
        if (xQueueReceive(Microbit_Receive_Queue, (void *)&event, (portTickType)portMAX_DELAY))
        {
            char messageCopy[UARTMESSAGELENGTH] = {0};

            memset(dtmp, 0, UARTMESSAGELENGTH * 8);

            //ESP_LOGI(TAG, "uart[%d] event:", BBC_UART_NUM);

            switch (event.type)
            {
            //Event of UART receving data
            /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
            case UART_DATA:
                //ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
                uart_read_bytes(BBC_UART_NUM, dtmp, event.size, portMAX_DELAY);
                //ESP_LOGI(TAG, "[DATA EVT]:");

                for (unsigned int i = 0; i < event.size; i++)
                {
                    messageCopy[i] = (char)dtmp[i];
                }

                // Serial.print("BBC_UART_NUM:");
                // Serial.println((const char *)dtmp);

                // Serial.print("Event Size:");
                // Serial.println(event.size);

                xQueueSend(Message_Queue, &messageCopy, portMAX_DELAY);

                break;
            //Event of HW FIFO overflow detected
            case UART_FIFO_OVF:
                //ESP_LOGI(TAG, "hw fifo overflow");
                // If fifo overflow happened, you should consider adding flow control for your application.
                // The ISR has already reset the rx FIFO,
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(BBC_UART_NUM);
                xQueueReset(Microbit_Receive_Queue);
                break;
            //Event of UART ring buffer full
            case UART_BUFFER_FULL:
                //ESP_LOGI(TAG, "ring buffer full");
                // If buffer full happened, you should consider encreasing your buffer size
                // As an example, we directly flush the rx buffer here in order to read more data.
                uart_flush_input(BBC_UART_NUM);
                xQueueReset(Microbit_Receive_Queue);
                break;
            //Event of UART RX break detected
            case UART_BREAK:
                //ESP_LOGI(TAG, "uart rx break");
                break;
            //Event of UART parity check error
            case UART_PARITY_ERR:
                //ESP_LOGI(TAG, "uart parity error");
                break;
            //Event of UART frame error
            case UART_FRAME_ERR:
                //ESP_LOGI(TAG, "uart frame error");
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH])
{
    //Serial.print("sendToMicrobit [msg]:");
    //Serial.println(msg);

    //the queue needs to work with a copy
    char queuedMsg[MAXBBCMESSAGELENGTH];
    strcpy(queuedMsg, msg);

    xQueueSend(Microbit_Transmit_Queue, &queuedMsg, portMAX_DELAY);
}

void microbit_transmit_task(void *pvParameters)
{
   // Serial.printf("Microbit TX task is on core %i\n", xPortGetCoreID());


    for (;;)
    {
        char msg[MAXBBCMESSAGELENGTH + 1] = {0};

        if (xQueueReceive(Microbit_Transmit_Queue, &msg, portMAX_DELAY))
        {
            //append # to the end so the microbit knows the end of the line
            strcat(msg, "#");

            int bytes = uart_write_bytes(BBC_UART_NUM, msg, strlen(msg));

            //Serial.printf(">> BBC TX: %s @ %i @ %i @ %i\n", msg, bytes, strlen(msg), millis());

             Serial.printf(">> BBC TX: %s\n", msg);

            //Added on 1/12/20 to try to stop flooding
            delay(10);
        }
    }

    vTaskDelete(NULL);
}
