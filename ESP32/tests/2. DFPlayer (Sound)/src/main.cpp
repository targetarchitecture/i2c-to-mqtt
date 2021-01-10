#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "SN7 pins.h"
#include <DFPlayerMini_Fast.h>

DFPlayerMini_Fast myMP3;
static QueueHandle_t led_evt_queue = NULL;
void MusicPlayer(void *pvParameters);
TaskHandle_t MusicTask;

void setup()
{
  Serial.begin(115200); //ESP32 USB Port
  Serial.flush();

  Serial1.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);

  led_evt_queue = xQueueCreate(1024, sizeof(char[10]));

  myMP3.begin(Serial1, 750);

  if (!myMP3.begin(Serial1))
  {
    Serial.println("DFPlayer error");

    delay(1000);
    ESP.restart();
  }

  delay(30);
  myMP3.volume(20);
  delay(30);

  int16_t v = myMP3.currentVolume();

  Serial.print("Volume:");
  Serial.println(v);

  int16_t fileCounts = myMP3.numSdTracks();

  if (fileCounts == -1)
  {
    Serial.println("DFPlayer no files found");

    delay(1000);
    ESP.restart();
  }
  else
  {
    Serial.print("Found ");
    Serial.print(fileCounts);
    Serial.println(" files");
  }

  xTaskCreatePinnedToCore(
      MusicPlayer, /* Task function. */
      "MP3 Task",  /* name of task. */
      10000,       /* Stack size of task */
      NULL,        /* parameter of the task */
      1,           /* priority of the task */
      &MusicTask,  /* Task handle to keep track of created task */
      1);          /* pin task to core 0 */

  delay(1000);

  long r = random(1, 12);
  char data[10];

  ltoa(r, data, 10);

  xQueueSend(led_evt_queue, &data, portMAX_DELAY);
}

void loop()
{
  delay(1000);
}

void MusicPlayer(void *pvParameters)
{
  char *data = (char *)malloc(1024);

  while (1)
  {
    int i = 1;
    memset(data, 0, 1024);

    //Block waiting for the first element of the receiving queue
    xQueueReceive(led_evt_queue, data, portMAX_DELAY);

    //Receive the second starting element in a loop, until 10 ticks have not received data
    while (xQueueReceive(led_evt_queue, data + i, 10))
    {
      i++;
    }

    std::string s = data;
    auto l = s.length();

    // remove trailing white space
    while (!s.empty() && std::isspace(s.back()))
    {
      s.pop_back();
    }

    //Analysis
    Serial.print("From Q:");
    Serial.print(s.c_str());
    Serial.print("<<");
    Serial.print(l);
    Serial.println("");

    long result = atol(s.c_str());

    myMP3.play(result);
  }
}
