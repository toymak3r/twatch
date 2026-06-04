#pragma once
#include "types.h"
void displayInit();
void renderClockPage(const ClockData& t, const DeviceState& d);
void renderSystemPage(const ClockData& t, const DeviceState& d);
void renderDatePage(const ClockData& t, const DeviceState& d);
void renderLowBattery(uint16_t mv);
