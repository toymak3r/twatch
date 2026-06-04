#pragma once
#include <stdint.h>
bool powerInit();                 // AXP2101 rails + button config; false if PMU missing
void powerCpuLow();               // 80 MHz
void setBacklight(uint8_t level); // 0 = panel+backlight off
uint16_t powerBattMv();
bool powerIsCharging();
void powerDisableTouchRail();     // ALDO3 off (for sleep)
void powerEnableTouchRail();      // ALDO3 on (on wake)
void powerReadClearIrq();         // read+clear AXP2101 IRQ status (deasserts INT pin)
void powerEnterDeepSleep();       // wake = power button (GPIO21)
extern uint8_t g_brightness;
