#include <Arduino.h>
#include "routing.h"

TaskHandle_t RoutingTask;

void routing_setup()
{
    xTaskCreatePinnedToCore(
        routing_task,                 /* Task function. */
        "Routing Task",               /* name of task. */
        configMINIMAL_STACK_SIZE * 4, /* Stack size of task */
        NULL,                         /* parameter of the task */
        2,                            /* priority of the task */
        &RoutingTask, 1);             /* Task handle to keep track of created task */
}

void routing_task(void *pvParameters)
{
    //char firstCharacter;

    /* Inspect our own high water mark on entering the task. */
    // BaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("routing_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    //  Serial.printf("Routing task is on core %i\n", xPortGetCoreID());

    for (;;)
    {
        // Serial.println("waiting for Command_Queue");

        // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        // Serial.print("routing_task uxTaskGetStackHighWaterMark:");
        // Serial.println(uxHighWaterMark);

        char msg[UARTMESSAGELENGTH] = {0};

        //wait for new music command in the queue
        xQueueReceive(Message_Queue, &msg, portMAX_DELAY);

        //Serial.printf ("Command_Queue: %s\n",msg);
        //Serial.println(msg);

        //TODO: Fix parsing by space
        //auto X = parseUART(msg, " ", false);

        auto X = parseUART(msg, "\r\n", false);

        // Serial.print("parseUART:");
        // Serial.println(X.size());

        for (int i = 0; i < X.size(); i++)
        {
            char cmd[MAXMESSAGELENGTH] = {0};

            strcpy(cmd, X[i].substr(0, MAXMESSAGELENGTH).c_str());

            Serial.printf("@@ BBC RX: %s\n", cmd);
            //Serial.println(cmd);

            //X[i].c_str()[0]

            if (cmd[0] == 'Z')
            {
                //Serial.println("Added to Music_Queue");

                xQueueSend(Music_Queue, &cmd, portMAX_DELAY);
            }
            else if (cmd[0] == 'Y')
            {
                //Serial.println("Added to Light_Queue");

                xQueueSend(Light_Queue, &cmd, portMAX_DELAY);
            }
            else if (cmd[0] == 'X')
            {
                //Serial.println("Added to DAC_Queue");

                xQueueSend(DAC_Queue, &cmd, portMAX_DELAY);
            }
            else if (cmd[0] == 'W')
            {
                //rotary encoders
                encoders_deal_with_message(cmd);
            }
            else if (cmd[0] == 'V')
            {
                //Serial.println("Added to Movement_Queue");

                xQueueSend(Movement_Queue, &cmd, portMAX_DELAY);
            }
            else if (cmd[0] == 'U')
            {
                //ADC encoders
                ADC_deal_with_message(cmd);
            }
            else if (cmd[0] == 'T')
            {
                //MQTT messaging
                xQueueSend(MQTT_Queue, &cmd, portMAX_DELAY);
            }
            else if (cmd[0] == '0')
            {
                //Nice....just notify the task to get on with it and send the UART message
              xTaskNotify(MicrobitTXTask, 0, eSetValueWithoutOverwrite);  
            }            
            else if (strcmp(cmd, "START") == 0)
            {
                //reboot ESP32
                //ESP.restart();
            }
        }
    }

    vTaskDelete(NULL);
}

std::vector<std::string> parseUART(const std::string &strStringToSplit,
                                   const std::string &strDelimiter,
                                   const bool keepEmpty)
{
    std::vector<std::string> vResult;
    if (strDelimiter.empty())
    {
        vResult.push_back(strStringToSplit);
        return vResult;
    }

    std::string::const_iterator itSubStrStart = strStringToSplit.begin(), itSubStrEnd;

    while (true)
    {
        itSubStrEnd = search(itSubStrStart, strStringToSplit.end(), strDelimiter.begin(), strDelimiter.end());

        std::string strTemp(itSubStrStart, itSubStrEnd);

        //removed as it was stopping spaces in SSIDs
        //remove whitespace
        //strTemp.erase(std::remove_if(strTemp.begin(), strTemp.end(), ::isspace), strTemp.end());

        if (keepEmpty || !strTemp.empty())
        {
            //only do this if we have a command
            if (strTemp.length() > 1)
            {
                // Serial.print(">>");
                // Serial.println(strTemp.c_str());

                vResult.push_back(strTemp);
            }
        }

        if (itSubStrEnd == strStringToSplit.end())
        {
            break;
        }

        itSubStrStart = itSubStrEnd + strDelimiter.size();
    }

    return vResult;
}
