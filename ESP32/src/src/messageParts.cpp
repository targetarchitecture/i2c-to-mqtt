#include <Arduino.h>
#include "messageParts.h"

messageParts processQueueMessage(std::string msg, std::string from)
{
  std::istringstream f(msg);
  std::string part;

  messageParts mParts = {};
  strcpy(mParts.fullMessage, msg.c_str());
  int index = 0;

  while (std::getline(f, part, ','))
  {
    if (index == 0)
    {
      strcpy(mParts.identifier, part.c_str());
    }
    if (index == 1)
    {
      strcpy(mParts.value1, part.c_str());
    }
    if (index == 2)
    {
      strcpy(mParts.value2, part.c_str());
    }
    if (index == 3)
    {
      strcpy(mParts.value3, part.c_str());
    }
    if (index == 4)
    {
      strcpy(mParts.value4, part.c_str());
    }
    if (index == 5)
    {
      strcpy(mParts.value5, part.c_str());
    }
    if (index == 6)
    {
      strcpy(mParts.value6, part.c_str());
    }
    if (index == 7)
    {
      strcpy(mParts.value7, part.c_str());
    }

    index++;
  }

  return mParts;
}
