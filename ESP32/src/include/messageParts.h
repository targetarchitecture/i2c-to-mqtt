#pragma once

#include <vector>
#include <sstream>
#include <iostream>

#include "defines.h"
#include "SN7 pins.h"

struct messageParts
{
    char fullMessage[MAXESP32MESSAGELENGTH];
    char identifier[20];
    char value1[MAXMESSAGEFRAGMENTSIZE];
    char value2[MAXMESSAGEFRAGMENTSIZE];
    char value3[MAXMESSAGEFRAGMENTSIZE];
    char value4[MAXMESSAGEFRAGMENTSIZE];
    char value5[MAXMESSAGEFRAGMENTSIZE];
    char value6[MAXMESSAGEFRAGMENTSIZE];
    char value7[MAXMESSAGEFRAGMENTSIZE];
};

messageParts processQueueMessage( std::string msg,  std::string from);

// struct messageParts2
// {
//     std::string fullMessage;
//     std::string identifier;
//     std::vector<std::string> values;
// };
