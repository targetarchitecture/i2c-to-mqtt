#pragma once

// #include <sstream>
// #include <iostream>
#include <string>
#include "defines.h"
#include "SN7 pins.h"

typedef struct
{
    char identifier[20];
    uint8_t value1;
    uint8_t value2;
    uint8_t value3;
    uint8_t value4;
    uint8_t value5;
    uint8_t value6;
    uint8_t value7;
} messageParts;

messageParts processQueueMessage(std::string msg);
