#pragma once
#include <stdint.h>
#include "gesture.h"
enum Page { PAGE_CLOCK, PAGE_SYSTEM, PAGE_DATE, PAGE_COUNT };
struct ClockData { int year, month, day, hour, minute, second, weekday, yearDay; };
struct DeviceState {
    uint16_t battMv; uint8_t battPct; bool charging;
    float coreTemp; uint32_t steps;
    bool syncOk; int syncAgeDays;
};
