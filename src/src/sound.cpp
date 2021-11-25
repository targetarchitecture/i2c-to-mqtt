#include <Arduino.h>
#include "sound.h"

/*
Mode 1: Files in root directory: Played in create date order, file names do not matter
Mode 2: Files in /01…/99 directories: Uses THREE digit file names (001.mp3)
Mode 3: Files in /mp3 directory: Uses FOUR digit files (0001.mp3) names played in that order (this is what I needed)
*/

DFRobotDFPlayerMini sound;

TaskHandle_t SoundTask;
TaskHandle_t SoundBusyTask;

const int commandPause = 50;

void sound_setup()
{
    //set-up the interupt
    pinMode(DFPLAYER_BUSY, INPUT);

    xTaskCreatePinnedToCore(
        sound_task,          /* Task function. */
        "Sound Task",        /* name of task. */
        3000,                /* Stack size of task (uxTaskGetStackHighWaterMark:11708) */
        NULL,                /* parameter of the task */
        sound_task_Priority, /* priority of the task */
        &SoundTask, 1);      /* Task handle to keep track of created task */

    xTaskCreatePinnedToCore(
        sound_busy_task,
        "Busy Task",
        3000,
        NULL,
        sound_busy_task_Priority,
        &SoundBusyTask,
        1);
}


void sound_busy_task(void *pvParameter)
{
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("busy_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    //int BusyPin = digitalRead(DFPLAYER_BUSY);

    int PreviousBusy = 1;

    for (;;)
    {
        int BusyPin = digitalRead(DFPLAYER_BUSY);

        if (BusyPin != PreviousBusy)
        {
            // Serial << "BusyPin=" << BusyPin << endl;

            if (BusyPin == LOW)
            {
                sendToMicrobit("SBUSY:1");
            }
            else
            {
                sendToMicrobit("SBUSY:0");
            }
        }

        PreviousBusy = BusyPin;

        delay(500);
    }

    vTaskDelete(NULL);
}

void sound_task(void *pvParameters)
{
    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("sound_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    //Configure serial port pins and busy pin
    pinMode(DFPLAYER_BUSY, INPUT);

    //set up UART
    Serial1.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);

    //Configure serial port pins and busy pin
    sound.begin(Serial1, true, true);
    sound.setTimeOut(750); //Set serial communication time out 750ms
    sound.outputDevice(DFPLAYER_DEVICE_SD);

    for (;;)
    {
        messageParts parts;

        //wait for new music command in the queue
        xQueueReceive(Sound_Queue, &parts, portMAX_DELAY);

        std::string identifier = parts.identifier;

        //Serial.print("xQueueReceive Sound_Queue:");
        //Serial.println(identifier.c_str());

        if (identifier.compare("SVOL") == 0)
        {
            auto volume = parts.value1;
            volume = constrain(volume, 0, 30);

            // Serial.print("vol:");
            // Serial.println(volume);

            delay(commandPause);
            sound.volume(volume);
            delay(commandPause);
        }
        if (identifier.compare("SFILECOUNT") == 0)
        {
            //Serial.println("HI");

            delay(commandPause);
            auto fileCount = sound.readFileCounts();
            delay(commandPause);

            //Serial << fileCount << endl;

            std::string requestMessage = "FILECOUNT:" + std::to_string(fileCount);

            //Serial << requestMessage.c_str() << endl;

            sendToMicrobit(requestMessage);
        }
        else if (identifier.compare("SPLAY") == 0)
        {
            //Serial << "identifier:" << identifier.c_str() << " folder:" << parts.value1 << " track:" << parts.value2 << endl;

            auto folderNum = parts.value1;
            folderNum = constrain(folderNum, 1, 99);

            auto trackNum = parts.value2;
            trackNum = constrain(trackNum, 1, 999);

            delay(commandPause);
            sound.playFolder(folderNum, trackNum);
            delay(commandPause);
        }
        else if (identifier.compare("SPAUSE") == 0)
        {
            delay(commandPause);
            sound.pause();
            delay(commandPause);
        }
        else if (identifier.compare("SRESUME") == 0)
        {
            delay(commandPause);
            sound.start();
            delay(commandPause);
        }
        else if (identifier.compare("SSTOP") == 0)
        {
            delay(commandPause);
            sound.stop();
            delay(commandPause);
        }
    }

    vTaskDelete(NULL);
}
