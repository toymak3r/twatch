#pragma once
#include "types.h"
void displayInit();
void renderClockPage(const ClockData& t, const DeviceState& d);
void renderSystemPage(const ClockData& t, const DeviceState& d);
void renderDatePage(const ClockData& t, const DeviceState& d);
void renderLowBattery(uint16_t mv);
void renderSyncSplash();  // FIX 4: shown during NTP sync so screen isn't black
