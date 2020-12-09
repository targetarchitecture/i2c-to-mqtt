#include <Arduino.h>
#include "music-dfplayer-dfrobot.h"
#include "DFRobotDFPlayerMini.h"

DFRobotDFPlayerMini sound;

TaskHandle_t MusicTask;
TaskHandle_t MusicBusyTask;

volatile int BusyPin;
const int commandPause = 50;

void music_setup()
{
    //Serial1.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);

    //Configure serial port pins and busy pin
    pinMode(DFPLAYER_BUSY, INPUT);
    // DFPlayer.begin(Serial1, 750);

    // DFPlayer.volume(20);
    // DFPlayer.play(8);

    //TODO: see if this is needed
    //     DFPlayer.reset();

    //   delay(30);
    //   DFPlayer.volume(20);
    //   delay(30);

    //   int16_t v = DFPlayer.currentVolume();

    //   Serial.print("Volume:");
    //   Serial.println(v);

    BusyPin = digitalRead(DFPLAYER_BUSY);

    //Serial.println("music_setup");

    xTaskCreatePinnedToCore(
        music_task,     /* Task function. */
        "Music Task",   /* name of task. */
        12000,          /* Stack size of task (uxTaskGetStackHighWaterMark:11708) */
        NULL,           /* parameter of the task */
        1,              /* priority of the task */
        &MusicTask, 1); /* Task handle to keep track of created task */

    xTaskCreatePinnedToCore(
        music_busy_task,    /* Task function. */
        "Music Busy Task",  /* name of task. */
        2048,               /* Stack size of task (uxTaskGetStackHighWaterMark:1756) */
        NULL,               /* parameter of the task */
        1,                  /* priority of the task */
        &MusicBusyTask, 1); /* Task handle to keep track of created task */
}

void music_busy_task(void *pvParameters)
{
    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("music_busy_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    int NewBusyPin;

    // Serial.printf("Music busy task is on core %i\n", xPortGetCoreID());

    for (;;)
    {
        //check for changes to busy pin
        NewBusyPin = digitalRead(DFPLAYER_BUSY);

        if (BusyPin != NewBusyPin)
        {
            // Serial.print("DFPlayer Busy:");
            // Serial.println(NewBusyPin);

            //TODO: Can this be improved?
            char msgtosend[MAXBBCMESSAGELENGTH];
            sprintf(msgtosend, "A1,%d", NewBusyPin);

            sendToMicrobit(msgtosend);

            // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            // Serial.print("music_busy_task uxTaskGetStackHighWaterMark:");
            // Serial.println(uxHighWaterMark);

            BusyPin = NewBusyPin;
        }

        delay(100);
    }

    vTaskDelete(NULL);
}

void music_task(void *pvParameters)
{
    //TODO: Ask Google if this is the best place to declare variables in an endless task
    messageParts parts;
    int16_t currentVolume;
    int16_t currentTrack;

    Serial1.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);

    //Configure serial port pins and busy pin
    sound.begin(Serial1, true, true);
    sound.setTimeOut(750); //Set serial communication time out 750ms
    sound.outputDevice(DFPLAYER_DEVICE_SD);

    /* Inspect our own high water mark on entering the task. */
    UBaseType_t uxHighWaterMark;
    uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    Serial.print("music_task uxTaskGetStackHighWaterMark:");
    Serial.println(uxHighWaterMark);

    // Serial.printf("Music task is on core %i\n", xPortGetCoreID());

    for (;;)
    {
        char msg[MAXMESSAGELENGTH] = {0};

        // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        // Serial.print("sound_task uxTaskGetStackHighWaterMark:");
        // Serial.println(uxHighWaterMark);

        //wait for new music command in the queue
        xQueueReceive(Music_Queue, &msg, portMAX_DELAY);

        // Serial.print("Music_Queue:");
        // Serial.println(msg);

        //TODO: see if need this copy of msg
        std::string X = msg;

        parts = processQueueMessage(X.c_str(), "MUSIC");

        // Serial.print("action:");
        // Serial.println(parts.identifier);

        if (strcmp(parts.identifier, "Z1") == 0)
        {
            auto volume = atoi(parts.value1);
            volume = constrain(volume, 0, 30);

            // Serial.print("vol:");
            // Serial.println(volume);

            delay(commandPause);
            sound.volume(volume);

            delay(commandPause);
            currentVolume = sound.readVolume();

            // Serial.print("current vol:");
            // Serial.println(currentVolume);

            char msgtosend[MAXBBCMESSAGELENGTH];
            sprintf(msgtosend, "A2,%i", currentVolume);

            sendToMicrobit(msgtosend);
        }
        else if (strcmp(parts.identifier, "Z2") == 0)
        {
            delay(commandPause);
            sound.volumeDown();
            delay(commandPause);
        }
        else if (strcmp(parts.identifier, "Z3") == 0)
        {
            delay(commandPause);
            sound.volumeUp();
            delay(commandPause);
        }
        else if (strcmp(parts.identifier, "Z4") == 0)
        {
            auto trackNum = atoi(parts.value1);
            trackNum = constrain(trackNum, 0, 2999);

            // Serial.print("DFPlayer.play:");
            // Serial.println(trackNum);

            // Serial.print("Serial1.getWriteError():");
            // Serial.println(Serial1.getWriteError());

            //Serial1.flush();

            delay(commandPause);
            sound.play(trackNum);
            delay(commandPause);
        }
        else if (strcmp(parts.identifier, "Z5") == 0)
        {
            delay(commandPause);
           sound.next();

            delay(commandPause);
            currentTrack = sound.readCurrentFileNumber();

            char msgtosend[MAXBBCMESSAGELENGTH];
            sprintf(msgtosend, "A3,%d", currentTrack);

            sendToMicrobit(msgtosend);
        }
        else if (strcmp(parts.identifier, "Z6") == 0)
        {
            delay(commandPause);
            sound.previous();

            delay(commandPause);
            currentTrack = sound.readCurrentFileNumber();

            char msgtosend[MAXBBCMESSAGELENGTH];
            sprintf(msgtosend, "A3,%d", currentTrack);

            sendToMicrobit(msgtosend);
        }
        else if (strcmp(parts.identifier, "Z7") == 0)
        {
            delay(commandPause);
            sound.pause();
            delay(commandPause);
        }
        else if (strcmp(parts.identifier, "Z8") == 0)
        {
            delay(commandPause);
            sound.start();
            delay(commandPause);
        }
        else if (strcmp(parts.identifier, "Z9") == 0)
        {
            delay(commandPause);
            sound.stop();
            delay(commandPause);
        }
    }

    vTaskDelete(NULL);
}