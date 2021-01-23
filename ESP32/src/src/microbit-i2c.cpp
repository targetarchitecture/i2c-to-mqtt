#include <Arduino.h>
#include "microbit-i2c.h"

TaskHandle_t Microbiti2cTask;

void microbit_i2c_setup()
{
    bool success = WireSlave1.begin(MICROBIT_SDA, MICROBIT_SCL, I2C_SLAVE_ADDR);

    if (!success)
    {
        Serial.println("I2C slave init failed");
        while (1)
            delay(100);
    }

    WireSlave1.onReceive(receiveEvent);
    WireSlave1.onRequest(requestEvent);

    pinMode(BBC_INT, INPUT_PULLUP);
    attachInterrupt(BBC_INT, handleBBCi2CInterupt, RISING);

    xTaskCreatePinnedToCore(
        i2c_rx_task,          /* Task function. */
        "i2c RX Task",        /* name of task. */
        8500,                 /* Stack size of task (uxTaskGetStackHighWaterMark: 8064) */
        NULL,                 /* parameter of the task */
        4,                    /* priority of the task */
        &Microbiti2cTask, 1); /* Task handle to keep track of created task */
}

void IRAM_ATTR handleBBCi2CInterupt()
{
    xTaskNotify(Microbiti2cTask, 0, eSetValueWithoutOverwrite);
}

void i2c_rx_task(void *pvParameter)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("i2c_rx_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    uint32_t ulNotifiedValue = 0;
    BaseType_t xResult;

    for (;;)
    {
        //10/12/20 - Just wait around to see if we get hailed to send
        xResult = xTaskNotifyWait(0X00, 0x00, &ulNotifiedValue, portMAX_DELAY);

        delay(1);

        WireSlave1.update();
    }
}

void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH])
{
    //Serial.print("sendToMicrobit [msg]:");
    //Serial.println(msg);

    //the queue needs to work with a copy
    char queuedMsg[MAXBBCMESSAGELENGTH];
    strcpy(queuedMsg, msg);

    xQueueSend(Microbit_Transmit_Queue, &queuedMsg, portMAX_DELAY);

    // Serial.print("sendToMicrobit [msg]:");
    // Serial.println(msg);
}

// function that executes whenever a complete and valid packet is received from BBC (i2c Master)
void receiveEvent(int howMany)
{
    std::string receivedMsg;

    while (1 < WireSlave1.available()) // loop through all but the last byte
    {
        char c = WireSlave1.read(); // receive byte as a character
        //Serial.print(c);            // print the character

        receivedMsg += c;
    }

    char c = WireSlave1.read(); // receive byte as an integer

    receivedMsg += c;

    //Serial.print("receivedMsg: ");
    //Serial.println(receivedMsg.c_str());

    //need to do something to copy the received message
    char queuedMsg[MAXBBCMESSAGELENGTH];
    strcpy(queuedMsg, receivedMsg.c_str());

    //Serial.print("strcpy: ");
    //Serial.println(queuedMsg);

    //disgard the 00 ones and not add them to the queue
    if (queuedMsg[0] != '0' && queuedMsg[1] != '0')
    {   
        //now add these to the routing queue for routing
        xQueueSend(Microbit_Receive_Queue, &queuedMsg, portMAX_DELAY);
    }
}

void requestEvent()
{
    char msg[MAXESP32MESSAGELENGTH]; // = {0};

    //wait for new BBC command in the queue
    if (xQueueReceive(Microbit_Transmit_Queue, &msg, 0))
    {
        // Serial.print("strlen: ");
        // Serial.println(strlen(msg));

        auto retval = WireSlave1.write(msg);

        // Serial.print("retval: ");
        // Serial.println(retval);

        // Serial.print("sent: ");
        // Serial.println(msg);
    }
    else
    {
        //just send back a blank string
        WireSlave1.print("");
    }
}