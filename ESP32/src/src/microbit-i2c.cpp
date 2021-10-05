#include <Arduino.h>
#include "microbit-i2c.h"

TaskHandle_t Microbiti2cTask;
SemaphoreHandle_t i2cSemaphore;

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
    //WireSlave1.onRequest(requestEvent);

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

        //WireSlave1.update();

        uint8_t inputBuffer[I2C_BUFFER_LENGTH] = {0};
        int16_t inputLen = 0;

        inputLen = i2c_slave_read_buffer(I2C_NUM_1, inputBuffer, I2C_BUFFER_LENGTH, 1);

        Serial << "inputLen: " << inputLen << endl;

        if (inputLen > 0)
        {
            std::string receivedMsg;

            for (size_t i = 0; i < inputLen; i++)
            {
                char c = inputBuffer[i]; // receive byte as a character

                receivedMsg += c;
            }

              Serial << "JRX: " << receivedMsg.c_str() << endl;
        }
    }
}

// function that executes whenever a complete and valid packet is received from BBC (i2c Master)
void receiveEvent(int howMany)
{
    std::string receivedMsg;

    while (1 < WireSlave1.available()) // loop through all but the last byte
    {
        char c = WireSlave1.read(); // receive byte as a character

        receivedMsg += c;
    }

    char c = WireSlave1.read(); // receive byte as an integer

    receivedMsg += c;

    //this bit here needs to set -up the message to send back
    dealWithMessage(receivedMsg);
}

void requestEvent()
{
    //  Serial << "TX: " << requestMessage.c_str() << endl;

    //WireSlave1.write("TEST");
}

void requestEvent(std::string message)
{
    uint8_t txBuffer[message.size()] = {0};

    std::copy(message.begin(), message.end(), txBuffer);

    i2c_reset_tx_fifo(I2C_NUM_1);
    i2c_slave_write_buffer(I2C_NUM_1, txBuffer, sizeof(txBuffer), 2000 / portTICK_RATE_MS); // 0);

    // for (size_t i = 0; i < sizeof(txBuffer); i++)
    // {
    //     Serial << "txBuffer[" << i << "]=" << txBuffer[i] << endl;
    // }
}

void dealWithMessage(std::string message)
{
    Serial << "RX: " << message.c_str() << endl;

    messageParts queuedMsg = processQueueMessage(message);

    std::string identifier = queuedMsg.identifier;

    //Serial << "queuedMsg: " << identifier.c_str() << endl;

    if (identifier.compare("RESTART") == 0)
    {
        //reboot ESP32...
        ESP.restart();
    }
    else if (identifier.compare("STARTING") == 0)
    {
        //clear down the queues
        xQueueReset(Sound_Queue);
        xQueueReset(Light_Queue);
        xQueueReset(DAC_Queue);
        xQueueReset(Movement_Queue);
        //xQueueReset(MQTT_Queue);
    }
    else if (identifier.compare("SVOL") == 0 ||
             identifier.compare("SPLAY") == 0 || identifier.compare("SPAUSE") == 0 ||
             identifier.compare("SRESUME") == 0 || identifier.compare("SSTOP") == 0)
    {
        xQueueSend(Sound_Queue, &queuedMsg, portMAX_DELAY);
    }
    else if (identifier.compare("SBUSY") == 0)
    {
        std::string requestMessage = std::to_string(digitalRead(DFPLAYER_BUSY));

        requestEvent(requestMessage);
    }
    else if (identifier.compare("LBLINK") == 0 || identifier.compare("LBREATHE") == 0 ||
             identifier.compare("LLEDONOFF") == 0 || identifier.compare("LLEDALLOFF") == 0 ||
             identifier.compare("LLEDALLON") == 0)
    {
        xQueueSend(Light_Queue, &queuedMsg, portMAX_DELAY);
    }
    else if (identifier.compare("DIAL1") == 0 || identifier.compare("DIAL2") == 0)
    {
        xQueueSend(DAC_Queue, &queuedMsg, portMAX_DELAY);
    }
    else if (identifier.compare("MSTOP") == 0 || identifier.compare("MANGLE") == 0 ||
             identifier.compare("MLINEAR") == 0 || identifier.compare("MSMOOTH") == 0 ||
             identifier.compare("MBOUNCY") == 0 || identifier.compare("MPWM") == 0)
    {
        xQueueSend(Movement_Queue, &queuedMsg, portMAX_DELAY);
    }
    else if (identifier.compare("ROTARY1") == 0)
    {
        std::string requestMessage = std::to_string(encoder1Count);

        requestEvent(requestMessage);
    }
    else if (identifier.compare("ROTARY2") == 0)
    {
        std::string requestMessage = std::to_string(encoder2Count);

        requestEvent(requestMessage);
    }
    else if (identifier.compare("SLIDER1") == 0)
    {
        std::string requestMessage = std::to_string(analogRead(ADC1));

        requestEvent(requestMessage);
    }
    else if (identifier.compare("SLIDER2") == 0)
    {
        std::string requestMessage = std::to_string(analogRead(ADC2));

        requestEvent(requestMessage);
    }
    else if (identifier.compare("SUPDATE") == 0)
    {
        std::string requestMessage = swithStates;

        requestEvent(requestMessage);
    }
    else if (identifier.compare("TUPDATE") == 0)
    {
        std::string requestMessage = touchStates;

        requestEvent(requestMessage);
    }
    else if (identifier.compare("TTHRSLD") == 0 || identifier.compare("TBOUNCE") == 0)
    {
        touch_deal_with_message(queuedMsg);
    }
}

messageParts processQueueMessage(std::string msg)
{
    std::istringstream f(msg);
    std::string part;

    messageParts mParts = {};
    int index = 0;

    while (std::getline(f, part, ','))
    {
        if (index == 0)
        {
            strcpy(mParts.identifier, part.c_str());
        }
        if (index == 1)
        {
            mParts.value1 = std::stoi(part);
        }
        if (index == 2)
        {
            mParts.value2 = std::stoi(part);
        }
        if (index == 3)
        {
            mParts.value3 = std::stoi(part);
        }
        if (index == 4)
        {
            mParts.value4 = std::stoi(part);
        }
        if (index == 5)
        {
            mParts.value5 = std::stoi(part);
        }
        if (index == 6)
        {
            mParts.value6 = std::stoi(part);
        }
        if (index == 7)
        {
            mParts.value7 = std::stoi(part);
        }

        index++;
    }

    return mParts;
}
