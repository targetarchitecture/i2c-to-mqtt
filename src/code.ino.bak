#include <Bluepad32.h>
#include <Wire.h>
#include <ArduinoLog.h>

void setup() {

  Serial.begin(115200);

  //LOG_LEVEL_VERBOSE
  //LOG_LEVEL_SILENT
  //LOG_LEVEL_TRACE
  Log.begin(LOG_LEVEL_SILENT, &Serial);

  setup_i2c();

  setup_dualsense();
}

void loop() {

  // This call fetches all the controllers' data.
  loop_dualsense();

  delay(5);
}
