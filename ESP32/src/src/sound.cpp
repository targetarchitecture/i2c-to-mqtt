#include <Arduino.h>
#include "sound.h"
#include "DFRobotDFPlayerMini.h"

DFRobotDFPlayerMini sound;

TaskHandle_t SoundTask;

const int commandPause = 50;

//QueueHandle_t Sound_Queue; //Queue to store all of the DFPlayer commands from the Microbit

void sound_setup()
{
    //Serial1.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);

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

    // Serial.printf("Music task is on core %i\n", xPortGetCoreID());

    for (;;)
    {
        char msg[MAXESP32MESSAGELENGTH] = {0};

        //wait for new music command in the queue
        xQueueReceive(Sound_Queue, &msg, portMAX_DELAY);

        //TODO: see if need this copy of msg
        std::string X = msg;

        parts = processQueueMessage(X, "MUSIC");

         //Serial.print("action:");
        // Serial.println(parts.identifier);

        if (strncmp(parts.identifier, "SVOL",4) == 0)
        {
            auto volume = std::stoi(parts.value1);
            volume = constrain(volume, 0, 30);

            // Serial.print("vol:");
            // Serial.println(volume);

            delay(commandPause);
            sound.volume(volume);
            delay(commandPause);

        }
        else if (strncmp(parts.identifier, "SPLAY",5) == 0)
        {
            auto trackNum = std::stoi(parts.value1);
            trackNum = constrain(trackNum, 0, 2999);

            delay(commandPause);
            sound.play(trackNum);
            delay(commandPause);
        }
        else if (strncmp(parts.identifier, "SPAUSE",6) == 0)
        {
            delay(commandPause);
            sound.pause();
            delay(commandPause);
        }
        else if (strncmp(parts.identifier, "SRESUME",7) == 0)
        {
            delay(commandPause);
            sound.start();
            delay(commandPause);
        }
        else if (strncmp(parts.identifier, "SSTOP",5) == 0)
        {
            delay(commandPause);
            sound.stop();
            delay(commandPause);
        }
    }

    vTaskDelete(NULL);
}

