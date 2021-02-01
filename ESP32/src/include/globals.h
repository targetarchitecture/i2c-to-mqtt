#pragma once

#include "messageParts.h"

SemaphoreHandle_t i2cSemaphore;

//const char *TAG = "rainbow_sparkle_unicorn";

messageParts processQueueMessage(const std::string msg, const std::string from);

// extern TaskHandle_t ADCTask;
// extern TaskHandle_t DACTask;
// extern TaskHandle_t LightTask;
// extern TaskHandle_t EncodersTask;
// extern TaskHandle_t MicrobitRXTask;
// extern TaskHandle_t SoundTask;
// extern TaskHandle_t MusicBusyTask;
// extern TaskHandle_t RoutingTask;
// extern TaskHandle_t TouchTask;
// extern TaskHandle_t MovementTask;

  char TXtoBBCmessage[MAXBBCMESSAGELENGTH];
  char RXfromBBCmessage[MAXESP32MESSAGELENGTH];