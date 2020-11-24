#ifndef messageParts_h
#define messageParts_h

#include "defines.h"
#include "SN4 pins.h"

struct messageParts
{
    char fullMessage[MAXMESSAGELENGTH];
    char identifier[4];
    char value1[MAXMESSAGEFRAGMENTSIZE];
    char value2[MAXMESSAGEFRAGMENTSIZE];
    char value3[MAXMESSAGEFRAGMENTSIZE];
    char value4[MAXMESSAGEFRAGMENTSIZE];
    char value5[MAXMESSAGEFRAGMENTSIZE];
    char value6[MAXMESSAGEFRAGMENTSIZE];
    char value7[MAXMESSAGEFRAGMENTSIZE];
};

#endif