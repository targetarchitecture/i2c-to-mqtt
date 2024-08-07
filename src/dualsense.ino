ControllerPtr myControllers[BP32_MAX_GAMEPADS];

void setup_dualsense() {
  Log.traceln("Firmware: %s", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Log.traceln("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  // Setup the Bluepad32 callbacks
  BP32.setup(&onConnectedController, &onDisconnectedController);

  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But it might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();

  // Enables mouse / touchpad support for gamepads that support them.
  // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
  // - First one: the gamepad
  // - Second one, which is a "virtual device", is a mouse.
  // By default, it is disabled.
  BP32.enableVirtualDevice(false);
}

bool loop_dualsense() {

  // This call fetches all the controllers' data.
  bool dataUpdated = BP32.update();

  // if (dataUpdated) {
  //  processControllers();
  // }

  // if (dataUpdated) {
  //   for (auto myController : myControllers) {
  //     if (myController && myController->isConnected()) {
  //       if (myController->isGamepad()) {
  //         //swap Y axis round so forward is postive
  //           int32_t correctedaxisRY = myController->axisRY() * -1;
  //         //calculateAxisHour(myController->axisRX(), correctedaxisRY);
  //         calculateMagnitude(myController->axisRX(), correctedaxisRY);
  //       }
  //     }
  //   }
  // }

  return dataUpdated;
}

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
  bool foundEmptySlot = false;
  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == nullptr) {
      Log.traceln("CALLBACK: Controller is connected, index=%d", i);
      // Additionally, you can get certain gamepad properties like:
      // Model, VID, PID, BTAddr, flags, etc.
      ControllerProperties properties = ctl->getProperties();
      Log.traceln("Controller model: %s, VID=0x%04x, PID=0x%04x", ctl->getModelName().c_str(), properties.vendor_id, properties.product_id);
      myControllers[i] = ctl;

//properties.btaddr

      foundEmptySlot = true;
      break;
    }
  }
  if (!foundEmptySlot) {
    Log.traceln("CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundController = false;

  for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
    if (myControllers[i] == ctl) {
      Log.traceln("CALLBACK: Controller disconnected from index=%d\n", i);
      myControllers[i] = nullptr;
      foundController = true;
      break;
    }
  }

  if (!foundController) {
    Log.traceln("CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

void dumpGamepad(ControllerPtr ctl) {
  Log.traceln(
    "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
    "misc: 0x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d",
    ctl->index(),        // Controller Index
    ctl->dpad(),         // D-pad
    ctl->buttons(),      // bitmask of pressed buttons
    ctl->axisX(),        // (-511 - 512) left X Axis
    ctl->axisY(),        // (-511 - 512) left Y axis
    ctl->axisRX(),       // (-511 - 512) right X axis
    ctl->axisRY(),       // (-511 - 512) right Y axis
    ctl->brake(),        // (0 - 1023): brake button
    ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
    ctl->miscButtons(),  // bitmask of pressed "misc" buttons
    ctl->gyroX(),        // Gyro X
    ctl->gyroY(),        // Gyro Y
    ctl->gyroZ(),        // Gyro Z
    ctl->accelX(),       // Accelerometer X
    ctl->accelY(),       // Accelerometer Y
    ctl->accelZ()        // Accelerometer Z
  );
}

// DualSense supports changing the colour LED.
void setColourLED(uint8_t red, uint8_t green, uint8_t blue) {

  for (auto myController : myControllers) {
    if (myController && myController->isConnected()) {
      if (myController->isGamepad()) {
        myController->setColorLED(red, green, blue);
      } else {
        Log.traceln("Unsupported controller (setColourLED)");
      }
    }
  }
}

// Turn on the 4 LED. Each bit represents one LED.
void setPlayerLEDs(int led) {

  for (auto myController : myControllers) {
    if (myController && myController->isConnected()) {
      if (myController->isGamepad()) {
        myController->setPlayerLEDs(led & 0x0f);
      } else {
        Log.traceln("Unsupported controller (setPlayerLEDs)");
      }
    }
  }
}

void playDualRumble() {

  for (auto myController : myControllers) {
    if (myController && myController->isConnected()) {
      if (myController->isGamepad()) {
        myController->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */, 0x40 /* strongMagnitude */);
      } else {
        Log.traceln("Unsupported controller (performDualRumble)");
      }
    }
  }
}
