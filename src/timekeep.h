#pragma once
#include "types.h"
bool rtcInit();              // PCF8563 over Wire (I2C bus 0)
bool readClock(ClockData& out);
bool ntpSync();              // WiFi up -> SNTP -> write PCF8563 -> WiFi off; true if set
