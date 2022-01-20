#include <Arduino.h>
#include "messaging.h"

void dealWithMessage(std::string message)
{
    message = message.substr(0, message.length() - 1);

    messageParts queuedMsg = processQueueMessage(message);

    std::string identifier = queuedMsg.identifier;

    //Serial << "dealWithMessage identifier: " << identifier.c_str() << endl;
    //Serial << "identifier.compare('LLEDALLON'): " << identifier.compare("LLEDALLON") << endl;

    if (identifier.compare("RESTART") == 0)
    {
        //Serial << "ESP.restart()" << endl;

        //reboot ESP32...
        ESP.restart();
    }
    else if (identifier.compare("SVOL") == 0 || identifier.compare("SFILECOUNT") == 0 ||
             identifier.compare("SPLAY") == 0 || identifier.compare("SPAUSE") == 0 ||
             identifier.compare("SRESUME") == 0 || identifier.compare("SSTOP") == 0)
    {
        //Serial << "Sound_Queue" << endl;

        xQueueSend(Sound_Queue, &queuedMsg, portMAX_DELAY);
    }
    else if (identifier.compare("SBUSY") == 0)
    {
        std::string requestMessage = "SBUSY:" + std::to_string(digitalRead(DFPLAYER_BUSY));

        sendToMicrobit(requestMessage);
    }
    else if (identifier.compare("LBLINK") == 0 || identifier.compare("LBREATHE") == 0 ||
             identifier.compare("LLEDONOFF") == 0 || identifier.compare("LLEDALLOFF") == 0 ||
             identifier.compare("LLEDALLON") == 0 || identifier.compare("LLEDINTENSITY") == 0)
    {
        xQueueSend(Light_Queue, &queuedMsg, portMAX_DELAY);
    }
    else if (identifier.compare("PUBLISH") == 0 || identifier.compare("SUBSCRIBE") == 0 ||
             identifier.compare("UNSUBSCRIBE") == 0)
    {
        xQueueSend(MQTT_Command_Queue, &queuedMsg, portMAX_DELAY);
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
        std::string requestMessage = "ROTARY1:" + std::to_string(encoder1Count);

        sendToMicrobit(requestMessage);
    }
    else if (identifier.compare("ROTARY2") == 0)
    {
        std::string requestMessage = "ROTARY2:" + std::to_string(encoder2Count);

        sendToMicrobit(requestMessage);
    }
    else if (identifier.compare("SLIDER1") == 0)
    {
        std::string requestMessage = "SLIDER1:" + std::to_string(analogRead(ADC1));

        sendToMicrobit(requestMessage);
    }
    else if (identifier.compare("SLIDER2") == 0)
    {
        std::string requestMessage = "SLIDER2:" + std::to_string(analogRead(ADC2));

        sendToMicrobit(requestMessage);
    }
    else if (identifier.compare("SSTATE") == 0)
    {
        std::string swithStates = "SSTATE:";

        for (size_t i = 0; i < 16; i++)
        {
            if (switchArray[i] == LOW)
            {
                swithStates.append("L");
            }
            else
            {
                swithStates.append("H");
            }
        }

        //Serial << swithStates.c_str() << endl;

        sendToMicrobit(swithStates);
    }
    else if (identifier.compare("TTHRSLD") == 0)
    {
        touch_deal_with_message(queuedMsg);
    }
    else if (identifier.compare("DEBUG") == 0)
    {
        Serial << queuedMsg.part1 << " " << queuedMsg.value1 << endl;
    }
    else if (identifier.compare("NVMSSID") == 0)
    {
        preferences.putString("ssid", queuedMsg.part1);
    }
    else if (identifier.compare("NVMPASSWORD") == 0)
    {
        preferences.putString("password",queuedMsg.part1);
    }
    else if (identifier.compare("NVMMQTTSERVER") == 0)
    {
        preferences.putString("mqtt_server", queuedMsg.part1);
    }
    else if (identifier.compare("NVMMQTTUSER") == 0)
    {
        preferences.putString("mqtt_user", queuedMsg.part1);
    }
    else if (identifier.compare("NVMMQTTPASSWORD") == 0)
    {
        preferences.putString("mqtt_password", queuedMsg.part1);
    }
}

messageParts processQueueMessage(std::string msg)
{
    //Serial << msg.length() << endl;

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
            try
            {
                // Serial << msg.c_str() << endl;
                // Serial << part.c_str() << endl;

                mParts.value1 = std::stoi(part);
            }
            catch (const std::exception &e)
            {
                strcpy(mParts.part1, part.c_str());

                //Serial << mParts.part1 << endl;
            }
        }
        if (index == 2)
        {
            try
            {
                mParts.value2 = std::stoi(part);
            }
            catch (const std::exception &e)
            {
                strcpy(mParts.part2, part.c_str());

                //Serial << mParts.part1 << endl;
            }
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
