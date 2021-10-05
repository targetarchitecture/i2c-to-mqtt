#pragma once

// #include <sstream>
// #include <iostream>
#include <string>
#include "defines.h"
#include "SN7 pins.h"

typedef struct
{
    char identifier[20];
    uint32_t value1;
    uint32_t value2;
    uint32_t value3;
    uint32_t value4;
    uint32_t value5;
    uint32_t value6;
    uint32_t value7;
} messageParts;

messageParts processQueueMessage(std::string msg);
