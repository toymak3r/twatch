#pragma once
#include <stdint.h>
bool pedoInit();        // BMA423 step counter; stays powered through deep sleep
uint32_t getSteps();
