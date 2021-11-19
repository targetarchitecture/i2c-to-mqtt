#include <Arduino.h>
#include <Preferences.h>

Preferences preferences;

void setup()
{
Serial.begin(9600);

  preferences.begin("SN9", false);

  preferences.putString("ssid", "the robot network");
  preferences.putString("password", "isaacasimov");

  preferences.putString("mqtt_server", "192.168.1.189");
  preferences.putString("mqtt_user", "public");
  preferences.putString("mqtt_password", "public");

  preferences.putUShort("tch_threshold", 12);
  preferences.putUShort("tch_release", 6);

  // Close the Preferences
  preferences.end();

  // Wait 10 seconds
  Serial.println("Restarting in 10 seconds...");
  delay(10000);

  // Restart ESP
  ESP.restart();
}

void loop()
{
  delay(1000);
}