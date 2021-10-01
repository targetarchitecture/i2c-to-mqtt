#include <Arduino.h>
#include "microbit-i2c.h"

TaskHandle_t Microbiti2cTask;

//char requestMessage[MAXESP32MESSAGELENGTH];

SemaphoreHandle_t i2cSemaphore;

std::string requestMessage;

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

// void sendToMicrobit(char msg[MAXBBCMESSAGELENGTH])
// {
//     //the queue needs to work with a copy
//     char queuedMsg[MAXBBCMESSAGELENGTH];
//     strcpy(queuedMsg, msg);

//     xQueueSend(Microbit_Transmit_Queue, &queuedMsg,  portMAX_DELAY);

//     //Serial.printf("Microbit_Transmit_Queue: %s\n", msg);
// }

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

    //this bit here needs to set -up the message to send back
    dealWithMessage(receivedMsg);

    //now add these to the routing queue for routing
    // xQueueSend(Microbit_Receive_Queue, &queuedMsg, portMAX_DELAY);
}

void requestEvent()
{
    WireSlave1.write(requestMessage.c_str());

    //just send back a blank string
    //  WireSlave1.print("");
}

void dealWithMessage(std::string message)
{

    char queuedMsg[MAXBBCMESSAGELENGTH];
    strcpy(queuedMsg, message.c_str());

    Serial.printf("dealWithMessage: %s\n", queuedMsg);

    if (strncmp(message.c_str(), "RESTART", 7) == 0)
    {
        //reboot ESP32...
        ESP.restart();
    }
    else if (strncmp(message.c_str(), "STARTING", 8) == 0)
    {
        //clear down the queues
        xQueueReset(Sound_Queue);
        xQueueReset(Light_Queue);
        xQueueReset(DAC_Queue);
        xQueueReset(Movement_Queue);
        //xQueueReset(MQTT_Queue);
    }
    else if (strncmp(message.c_str(), "SVOL", 4) == 0 ||
             strncmp(message.c_str(), "SPLAY", 5) == 0 || strncmp(message.c_str(), "SPAUSE", 6) == 0 ||
             strncmp(message.c_str(), "SRESUME", 7) == 0 || strncmp(message.c_str(), "SSTOP", 5) == 0)
    {
        xQueueSend(Sound_Queue, &queuedMsg, portMAX_DELAY);
    }
    else if (strncmp(message.c_str(), "SBUSY", 5) == 0)
    {

       // digitalRead(DFPLAYER_BUSY);
        requestMessage = std::to_string(digitalRead(DFPLAYER_BUSY));
    }
    else if (strncmp(message.c_str(), "LBLINK", 6) == 0 || strncmp(message.c_str(), "LBREATHE", 8) == 0 ||
             strncmp(message.c_str(), "LLEDONOFF", 9) == 0 || strncmp(message.c_str(), "LRESET", 6) == 0 ||
             strncmp(message.c_str(), "LLEDALLON", 9) == 0)
    {
        xQueueSend(Light_Queue, &queuedMsg, portMAX_DELAY);
    }
    else if (strncmp(message.c_str(), "DIAL1", 5) == 0 || strncmp(message.c_str(), "DIAL2", 5) == 0)
    {
        xQueueSend(DAC_Queue, &queuedMsg, portMAX_DELAY);
    }
    else if (strncmp(message.c_str(), "MSTOP", 5) == 0 || strncmp(message.c_str(), "MANGLE", 6) == 0 ||
             strncmp(message.c_str(), "MLINEAR", 7) == 0 || strncmp(message.c_str(), "MSMOOTH", 7) == 0 ||
             strncmp(message.c_str(), "MBOUNCY", 7) == 0 || strncmp(message.c_str(), "MPWM", 4) == 0)
    {
        xQueueSend(Movement_Queue, &queuedMsg, portMAX_DELAY);
    }
    else if (strncmp(message.c_str(), "ROTARY1", 7) == 0)
    {
        requestMessage = std::to_string(encoder1Count);
    }
    else if (strncmp(message.c_str(), "ROTARY2", 7) == 0)
    {
        requestMessage = std::to_string(encoder2Count);
    }
    else if (strncmp(message.c_str(), "SLIDER1", 7) == 0)
    {
        requestMessage = std::to_string(analogRead(ADC1));
    }
    else if (strncmp(message.c_str(), "SLIDER2", 7) == 0)
    {
        requestMessage = std::to_string(analogRead(ADC2));
    }
    else if (strncmp(message.c_str(), "SUPDATE", 7) == 0)
    {
        requestMessage = swithStates;
    }
    else if (strncmp(message.c_str(), "TUPDATE", 7) == 0)
    {
        requestMessage = touchStates;
    }
    else if (strncmp(message.c_str(), "TTHRSLD", 7) == 0 || strncmp(message.c_str(), "TBOUNCE", 7) == 0)
    {
        touch_deal_with_message(message.c_str());
    }

    // xQueueSend(MQTT_Queue, &cmd, portMAX_DELAY);
}