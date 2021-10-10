#include <Arduino.h>
#include "sound.h"
#include "DFRobotDFPlayerMini.h"

DFRobotDFPlayerMini sound;

TaskHandle_t SoundTask;

const int commandPause = 50;

void sound_setup()
{
    //Configure serial port pins and busy pin
    pinMode(DFPLAYER_BUSY, INPUT);

    int BusyPin = digitalRead(DFPLAYER_BUSY);

    xTaskCreatePinnedToCore(
        sound_task,          /* Task function. */
        "Sound Task",        /* name of task. */
        12000,               /* Stack size of task (uxTaskGetStackHighWaterMark:11708) */
        NULL,                /* parameter of the task */
        sound_task_Priority, /* priority of the task */
        &SoundTask, 1);      /* Task handle to keep track of created task */
}

void sound_task(void *pvParameters)
{
    //TODO: Ask Google if this is the best place to declare variables in an endless task
    messageParts parts;

    Serial1.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);

    //Configure serial port pins and busy pin
    sound.begin(Serial1, true, true);
    sound.setTimeOut(750); //Set serial communication time out 750ms
    sound.outputDevice(DFPLAYER_DEVICE_SD);

    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("music_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    for (;;)
    {
        messageParts parts;

        //wait for new music command in the queue
        xQueueReceive(Sound_Queue, &parts, portMAX_DELAY);

        std::string identifier = parts.identifier;

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
        else if (identifier.compare("SPLAY") == 0)
        {
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
