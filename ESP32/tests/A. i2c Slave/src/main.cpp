#include <Arduino.h>
#include <Wire.h>
#include <WireSlave.h>
#include <SN4 pins.h>
#include <WirePacker.h>

void i2cScan();
void receiveEvent(int howMany);
void requestEvent();

#define SLAVE_SDA_PIN 17
#define SLAVE_SCL_PIN 16
#define I2C_SLAVE_ADDR 4

void setup()
{
  Serial.begin(115200);

  pinMode(2, OUTPUT);

  Wire.begin(SDA, SCL); //I2C bus

  bool success = WireSlave1.begin(SLAVE_SDA_PIN, SLAVE_SCL_PIN, I2C_SLAVE_ADDR);

  if (!success)
  {
    Serial.println("I2C slave init failed");
    while (1)
      delay(100);
  }

  WireSlave1.onReceive(receiveEvent);
  WireSlave1.onRequest(requestEvent);

  Serial.println("I2C Scanner");
}

static unsigned long lastWireTransmit = 0;
static byte x = 0;

void loop()
{
  WireSlave1.update();

  // let I2C and other ESP32 peripherals interrupts work
  delay(1);
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
  
  // Serial.println("WireSlave1.print");
  // Serial.println(millis());
}