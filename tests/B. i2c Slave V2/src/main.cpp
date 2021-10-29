#include <Arduino.h>
#include <Wire.h>
#include <WireSlave.h>
#include <SN7 pins.h>
#include <WirePacker.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include <stdio.h>
#include "soc/i2c_reg.h"

void receiveEvent(int howMany);
void requestEvent();
void IRAM_ATTR isr_i2c_from_microbit();
void i2c_rx_task(void *pvParameter);

#define I2C_SLAVE_ADDR 4

QueueHandle_t MQTT_Queue;
TaskHandle_t SwitchTask;

void setup()
{
  Serial.begin(115200);

  pinMode(2, OUTPUT);

  Wire.begin(SDA, SCL); //I2C bus

  bool success = WireSlave1.begin(MICROBIT_SDA, MICROBIT_SCL, I2C_SLAVE_ADDR);

  if (!success)
  {
    Serial.println("I2C slave init failed");
    while (1)
      delay(100);
  }

  WireSlave1.onReceive(receiveEvent);
  WireSlave1.onRequest(requestEvent);

  pinMode(BBC_INT, INPUT_PULLUP);
  attachInterrupt(BBC_INT, isr_i2c_from_microbit, RISING);

  MQTT_Queue = xQueueCreate(50, sizeof(uint8_t));

  xTaskCreatePinnedToCore(
      i2c_rx_task,     /* Task function. */
      "i2c RX Task",   /* name of task. */
      8500,            /* Stack size of task (uxTaskGetStackHighWaterMark: 8200) */
      NULL,            /* parameter of the task */
      1,               /* priority of the task */
      &SwitchTask, 1); /* Task handle to keep track of created task */
}

void IRAM_ATTR isr_i2c_from_microbit()
{
  int32_t cmd = 1;

  xQueueSend(MQTT_Queue, &cmd, portMAX_DELAY);
}

void i2c_rx_task(void *pvParameter)
{
  // UBaseType_t uxHighWaterMark;
  // uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  // Serial.print("i2c_rx_task uxTaskGetStackHighWaterMark:");
  // Serial.println(uxHighWaterMark);

  int32_t cmd = 0;

  for (;;)
  {
    digitalWrite(2, LOW);

    xQueueReceive(MQTT_Queue, &cmd, portMAX_DELAY);

    digitalWrite(2, HIGH);

    WireSlave1.update();
  }
}

void loop()
{
  delay(1000);
}

// function that executes whenever a complete and valid packet
// is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany)
{
  digitalWrite(2, HIGH);

  Serial.println(millis());

  while (1 < WireSlave1.available()) // loop through all but the last byte
  {
    char c = WireSlave1.read(); // receive byte as a character
    Serial.print(c);            // print the character
  }

  char c = WireSlave1.read(); // receive byte as an integer
  Serial.println(c);

  Serial.println(millis());

  digitalWrite(2, LOW);
}

void requestEvent()
{
  WireSlave1.printf("hello @ %i", millis());
}