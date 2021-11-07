#pragma once

#include "defines.h"
#include "SN9 pins.h"
#include <ESP32Encoder.h>

void encoders_setup();
void encoders_task(void *pvParameter);
