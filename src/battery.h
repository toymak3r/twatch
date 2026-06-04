#pragma once
#include <stdint.h>
// Li-ion 4200mV=100% .. 3300mV=0%, clamped, linear.
inline uint8_t batteryPercent(uint16_t mv) {
    if (mv >= 4200) return 100;
    if (mv <= 3300) return 0;
    return (uint8_t)(((uint32_t)(mv - 3300) * 100) / 900);
}
