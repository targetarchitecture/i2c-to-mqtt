#ifndef globals_h
#define globals_h

#include "messageParts.h"

SemaphoreHandle_t i2cSemaphore;

//const char *TAG = "rainbow_sparkle_unicorn";

messageParts processQueueMessage(const std::string msg, const std::string from);

int printToSerial(const char *format, ...);

//extern volatile servo servos[15];

extern TaskHandle_t ADCTask;
extern TaskHandle_t DACTask;
extern TaskHandle_t LightTask;
extern TaskHandle_t EncodersTask;
extern TaskHandle_t MicrobitRXTask;
extern TaskHandle_t MicrobitTXTask;
extern TaskHandle_t MusicTask;
extern TaskHandle_t MusicBusyTask;
extern TaskHandle_t RoutingTask;
extern TaskHandle_t TouchTask;
extern TaskHandle_t MovementTask;

#endif