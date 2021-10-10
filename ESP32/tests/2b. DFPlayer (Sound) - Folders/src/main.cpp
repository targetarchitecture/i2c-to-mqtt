#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "SN7 pins.h"
#include "DFRobotDFPlayerMini.h"

DFRobotDFPlayerMini sound;

void setup()
{
  Serial.begin(115200); //ESP32 USB Port
  Serial.flush();

  Serial1.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);

  sound.begin(Serial1, 750);

  if (!sound.begin(Serial1))
  {
    Serial.println("DFPlayer error");

    delay(1000);
    ESP.restart();
  }

  delay(30);
  sound.volume(20);
  delay(30);

  int16_t v = sound.readVolume();

  Serial.print("Volume:");
  Serial.println(v);

  int16_t fileCounts = sound.readFileCounts();

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

sound.playFolder(99,1);

}

void loop()
{
  delay(1000);
}
