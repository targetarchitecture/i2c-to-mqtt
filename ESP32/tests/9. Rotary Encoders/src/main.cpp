#include <Arduino.h>
#include <ESP32Encoder.h>
#include "SN4 pins.h"

ESP32Encoder encoder1;
ESP32Encoder encoder2;
volatile int32_t encoder1Count = 0;
volatile int32_t encoder2Count = 0;
TaskHandle_t EncodersTask;

void encoders_task(void *pvParameter);

void setup()
{
  Serial.begin(115200); //ESP32 USB Port

  // Enable the weak pull up resistors
  ESP32Encoder::useInternalWeakPullResistors = UP;

  // Attach pins for use as encoder pins
  encoder1.attachHalfQuad(ROTARY1CK, ROTARY1DT); //SN4
  encoder1.clearCount();
  encoder1.setCount(0);

  // Attach pins for use as encoder pins
  encoder2.attachFullQuad(ROTARY2CK, ROTARY2DT); //SN4
  encoder2.clearCount();
  encoder2.setCount(0);

  //start task to read an send rotary information every 500ms
  xTaskCreate(&encoders_task, "encoders_task", 2048, NULL, 0, NULL);
}

void encoders_task(void *pvParameter)
{
  std::string msg;
  int32_t newEncoder1Count;
  int32_t newEncoder2Count;

  while (1)
  {
    newEncoder1Count = encoder1.getCount();
    newEncoder2Count = encoder2.getCount();

    if (newEncoder1Count != encoder1Count)
    {
      msg = "D1,";

      if (newEncoder1Count > encoder1Count)
      {
        msg.append("+");
      }
      else
      {
        msg.append("-");
      }

      Serial.println("Encoder 1 = " + String(newEncoder1Count) + " direction = " + msg.c_str());

      encoder1Count = newEncoder1Count;
    }

    if (newEncoder2Count != encoder2Count)
    {
      msg = "D2,";

      if (newEncoder2Count > encoder2Count)
      {
        msg.append("+");
      }
      else
      {
        msg.append("-");
      }

      Serial.println("Encoder 2 = " + String(newEncoder2Count) + " direction = " + msg.c_str());

      encoder2Count = newEncoder2Count;
    }

    delay(50);
  }
}

void loop()
{
  delay(1000);
}
