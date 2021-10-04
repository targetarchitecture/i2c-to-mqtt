#pragma once

#include <sstream>
#include <iostream>
#include <string>
#include "defines.h"
#include "SN7 pins.h"

// struct messageParts
// {
//     char fullMessage[MAXESP32MESSAGELENGTH];
//     char identifier[20];
//     char value1[MAXMESSAGEFRAGMENTSIZE];
//     char value2[MAXMESSAGEFRAGMENTSIZE];
//     char value3[MAXMESSAGEFRAGMENTSIZE];
//     char value4[MAXMESSAGEFRAGMENTSIZE];
//     char value5[MAXMESSAGEFRAGMENTSIZE];
//     char value6[MAXMESSAGEFRAGMENTSIZE];
//     char value7[MAXMESSAGEFRAGMENTSIZE];
// };

struct messageParts
{
    std::string fullMessage;
    std::string identifier;
    std::string value1;
    std::string value2;
    std::string value3;
    std::string value4;
    std::string value5;
    std::string value6;
    std::string value7;
};


//using namespace std;

// struct messageParts2
// {
//     string fullMessage;
//     char identifier[20];
//     char value1[MAXMESSAGEFRAGMENTSIZE];
//     char value2[MAXMESSAGEFRAGMENTSIZE];
//     char value3[MAXMESSAGEFRAGMENTSIZE];
//     char value4[MAXMESSAGEFRAGMENTSIZE];
//     char value5[MAXMESSAGEFRAGMENTSIZE];
//     char value6[MAXMESSAGEFRAGMENTSIZE];
//     char value7[MAXMESSAGEFRAGMENTSIZE];
// };

messageParts processQueueMessage(std::string msg, std::string from);

