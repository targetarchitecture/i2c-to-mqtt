#include <Arduino.h>
#include <Wire.h>
#include "DFRobotDFPlayerMini.h"
#include "pins.h"

DFRobotDFPlayerMini sound;

void i2cScan();

void setup()
{
  Wire.begin(SDA, SCL); //I2C bus

  Serial.begin(115200); //ESP32 USB Port
  Serial.println("Configure the UARTs");

  //Configure serial port pins and busy pin
  pinMode(DFPLAYER_BUSY, INPUT);
  Serial1.begin(9600, SERIAL_8N1, DFPLAYER_RX, DFPLAYER_TX);

  //BBC microbit serial setup
  Serial2.begin(115200, SERIAL_8N1, MICROBIT_RX, MICROBIT_TX);

  //look for i2c devices
  i2cScan();

  if (!sound.begin(Serial1, true, true))
  {
    Serial.println("DFPlayer error");

    delay(5000);
    ESP.restart();
  }

  Serial.print("DFPlayer Busy:");
  Serial.println(digitalRead(DFPLAYER_BUSY));

  delay(30);
  sound.setTimeOut(750); //Set serial communication time out 750ms
  delay(30);

  sound.outputDevice(DFPLAYER_DEVICE_SD);

  delay(30);

  int fileCounts = sound.readFileCounts();

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

    sound.volume(30);
    Serial.print("Volume: ");
    Serial.println(sound.readVolume());
  }
}

void loop()
{
  long rnd = random(10);

  Serial2.println(rnd);
  Serial.println(rnd);

  String received = "";

  while (Serial2.available())
  {
    received = Serial2.readStringUntil('\n');

    Serial.print("play:");
    Serial.println(received.toInt());

    //play the mp3
    sound.play(received.toInt());
    delay(30);
  }

  delay(1000);
}

void i2cScan()
{
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
      {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknow error at address 0x");
      if (address < 16)
      {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
  {
    Serial.println("No I2C devices found\n");
  }
  else
  {
    Serial.println("I2C scan complete");
  }
}
