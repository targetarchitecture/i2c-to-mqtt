void setup_i2c() {
  Wire.begin(121);                  // Join I2C bus as the slave with address 1
  Wire.onReceive(ReceivedCommand);  // When the data transmission is detected call receiveEvent function
  Wire.onRequest(SendData);
}

//String requestDataFromMicrobit;

/*Function/Event call****************************************************************************/
void ReceivedCommand(int howMany) {

  String command;

  Log.trace("ReceivedCommand:");

  while (Wire.available()) {
    char c = Wire.read();
    command.concat(c);
  }

  Log.traceln(command.c_str());

  if (command.startsWith("RUMBLE")) {
    playDualRumble();  //Perform controller rumble
  } else if (command.startsWith("LED:")) {

    int Led;

    sscanf(command.c_str(), "LED:%d", &Led);

    Log.traceln("setPlayerLEDs:%d", Led);

    setPlayerLEDs(Led);  //Turn on the 4 LED. Each bit represents one LED.

  } else if (command.startsWith("COLOUR:")) {

    uint8_t r, g, b;

    sscanf(command.c_str(), "COLOUR:%u,%u,%u", &r, &g, &b);

    Log.traceln("COLOUR:%u,%u,%u", r, g, b);

    setColourLED(r, g, b);  //Turn on the colour LEDs
  }
  //else {
  //   requestDataFromMicrobit = command;
  //   Log.traceln("requestDataFromMicrobit:%s", command);
  // }
}

int whichDataToSend = 0;

void SendData() {

  char txt[32];  // respond with message of 32 bytes (the max wire.h supports)

  for (auto myController : myControllers) {
    if (myController && myController->isConnected()) {
      if (myController->isGamepad()) {

        if (whichDataToSend == 0) {

          bool DPAD_UP_PRESSED = 0;
          bool DPAD_UP_RIGHT_PRESSED = 0;
          bool DPAD_RIGHT_PRESSED = 0;
          bool DPAD_DOWN_RIGHT_PRESSED = 0;
          bool DPAD_DOWN_PRESSED = 0;
          bool DPAD_DOWN_LEFT_PRESSED = 0;
          bool DPAD_LEFT_PRESSED = 0;
          bool DPAD_UP_LEFT_PRESSED = 0;

          if (myController->dpad() == DPAD_UP) { DPAD_UP_PRESSED = true; }
          if (myController->dpad() == DPAD_UP + DPAD_RIGHT) { DPAD_UP_RIGHT_PRESSED = true; }
          if (myController->dpad() == DPAD_RIGHT) { DPAD_RIGHT_PRESSED = true; }
          if (myController->dpad() == DPAD_DOWN + DPAD_RIGHT) { DPAD_DOWN_RIGHT_PRESSED = true; }
          if (myController->dpad() == DPAD_DOWN) { DPAD_DOWN_PRESSED = true; }
          if (myController->dpad() == DPAD_DOWN + DPAD_LEFT) { DPAD_DOWN_LEFT_PRESSED = true; }
          if (myController->dpad() == DPAD_LEFT) { DPAD_LEFT_PRESSED = true; }
          if (myController->dpad() == DPAD_UP + DPAD_LEFT) { DPAD_UP_LEFT_PRESSED = true; }

          snprintf(txt, sizeof txt, "DPAD,%d,%d,%d,%d,%d,%d,%d,%d\n",
                   DPAD_UP_PRESSED,
                   DPAD_UP_RIGHT_PRESSED,
                   DPAD_RIGHT_PRESSED,
                   DPAD_DOWN_RIGHT_PRESSED,
                   DPAD_DOWN_PRESSED,
                   DPAD_DOWN_LEFT_PRESSED,
                   DPAD_LEFT_PRESSED,
                   DPAD_UP_LEFT_PRESSED);

        } else if (whichDataToSend == 1) {
          snprintf(txt, sizeof txt, "BUTTONS,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                   myController->a(),
                   myController->b(),
                   myController->x(),
                   myController->y(),
                   myController->l1(),
                   myController->l2(),
                   myController->r1(),
                   myController->r2(),
                   myController->thumbL(),
                   myController->thumbR());

        } else if (whichDataToSend == 2) {
          //swap Y axis round so forward is postive
          int32_t correctedaxisLY = myController->axisY() * -1;
          snprintf(txt, sizeof txt, "AXISL,%i,%i,%i,%i\n", myController->axisX(), correctedaxisLY, calculateAxisHour(myController->axisX(), correctedaxisLY), calculateMagnitude(myController->axisX(), correctedaxisLY));
        } else if (whichDataToSend == 3) {
          //swap Y axis round so forward is postive
          int32_t correctedaxisRY = myController->axisRY() * -1;
          snprintf(txt, sizeof txt, "AXISR,%i,%i,%i,%i\n", myController->axisRX(), correctedaxisRY, calculateAxisHour(myController->axisRX(), correctedaxisRY), calculateMagnitude(myController->axisRX(), correctedaxisRY));
        } else if (whichDataToSend == 4) {
          snprintf(txt, sizeof txt, "TRIGGERS,%i,%i\n", myController->brake(), myController->throttle());
        } else if (whichDataToSend == 5) {
          snprintf(txt, sizeof txt, "GYRO,%i,%i,%i\n", myController->gyroX(), myController->gyroY(), myController->gyroZ());
        } else if (whichDataToSend == 6) {
          snprintf(txt, sizeof txt, "ACCEL,%i,%i,%i\n", myController->accelX(), myController->accelY(), myController->accelZ());
        }

        Log.trace("sendData:%s", txt);
        Wire.write(txt);

        whichDataToSend++;

        if (whichDataToSend > 6) {
          whichDataToSend = 0;
        }
      }
    }
  }
}
