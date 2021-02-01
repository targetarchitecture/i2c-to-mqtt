#include <Arduino.h>
#include "sound.h"
#include "DFRobotDFPlayerMini.h"

DFRobotDFPlayerMini sound;

TaskHandle_t SoundTask;
TaskHandle_t SoundBusyTask;

volatile int BusyPin;
const int commandPause = 50;

QueueHandle_t Sound_Queue; //Queue to store all of the DFPlayer commands from the Microbit

void sound_setup()
{
    //Serial1.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);

    //Configure serial port pins and busy pin
    pinMode(DFPLAYER_BUSY, INPUT);

    //set-up the interupt
    attachInterrupt(DFPLAYER_BUSY, handleSoundInterupt, CHANGE);

    BusyPin = digitalRead(DFPLAYER_BUSY);

    Sound_Queue = xQueueCreate(50, sizeof(RXfromBBCmessage));

    //Serial.println("music_setup");

    xTaskCreatePinnedToCore(
        sound_task,          /* Task function. */
        "Sound Task",        /* name of task. */
        12000,               /* Stack size of task (uxTaskGetStackHighWaterMark:11708) */
        NULL,                /* parameter of the task */
        sound_task_Priority, /* priority of the task */
        &SoundTask, 1);      /* Task handle to keep track of created task */

    xTaskCreatePinnedToCore(
        sound_busy_task,          /* Task function. */
        "Sound Busy Task",        /* name of task. */
        2048,                     /* Stack size of task (uxTaskGetStackHighWaterMark:1756) */
        NULL,                     /* parameter of the task */
        sound_busy_task_Priority, /* priority of the task */
        &SoundBusyTask, 1);       /* Task handle to keep track of created task */
}

void sound_busy_task(void *pvParameters)
{
    /* Inspect our own high water mark on entering the task. */
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("music_busy_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    uint32_t ulNotifiedValue = 0;
    BaseType_t xResult;

    int NewBusyPin;

    // Serial.printf("Music busy task is on core %i\n", xPortGetCoreID());

    for (;;)
    {
        //wait for value to change
        xResult = xTaskNotifyWait(0X00, 0x00, &ulNotifiedValue, portMAX_DELAY);

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

        //no need for delay aS it now waits for the interupt (13.1.21)
        //delay(100);
    }

    vTaskDelete(NULL);
}

void sound_task(void *pvParameters)
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
    // UBaseType_t uxHighWaterMark;
    // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
    // Serial.print("music_task uxTaskGetStackHighWaterMark:");
    // Serial.println(uxHighWaterMark);

    // Serial.printf("Music task is on core %i\n", xPortGetCoreID());

    for (;;)
    {
        char msg[MAXESP32MESSAGELENGTH] = {0};

        // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
        // Serial.print("sound_task uxTaskGetStackHighWaterMark:");
        // Serial.println(uxHighWaterMark);

        //wait for new music command in the queue
        xQueueReceive(Sound_Queue, &msg, portMAX_DELAY);

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

            currentVolume = constrain(currentVolume, 0, 30);

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

void IRAM_ATTR handleSoundInterupt()
{
    //int32_t cmd = 1;

    xTaskNotify(SoundBusyTask, 0, eSetValueWithoutOverwrite);
}