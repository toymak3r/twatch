#include "power.h"
#include "pins.h"
#include <Wire.h>
#include <XPowersLib.h>
#include <esp_sleep.h>
#include <esp32-hal-ledc.h>
#include <esp32-hal-cpu.h>

static XPowersAXP2101 PMU;
uint8_t g_brightness = 0;

bool powerInit() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (!PMU.init(Wire, I2C_SDA_PIN, I2C_SCL_PIN)) return false;

    PMU.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);
    PMU.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_900MA);
    PMU.setSysPowerDownVoltage(2600);

    PMU.setALDO1Voltage(3300);  // RTC VBAT
    PMU.setALDO2Voltage(3300);  // TFT backlight/panel VDD
    PMU.setALDO3Voltage(3300);  // touch VDD

    PMU.enableALDO1();
    PMU.enableALDO2();
    PMU.enableALDO3();

    // Unused rails OFF for battery: radio, haptics, GPS, and all spare DC/DLDO.
    PMU.disableALDO4();   // radio
    PMU.disableBLDO2();   // DRV2605 haptics
    PMU.disableDC3();     // GPS
    PMU.disableDC2(); PMU.disableDC4(); PMU.disableDC5();
    PMU.disableBLDO1(); PMU.disableCPUSLDO();
    PMU.disableDLDO1(); PMU.disableDLDO2();

    PMU.setPowerKeyPressOffTime(XPOWERS_POWEROFF_4S);
    PMU.setPowerKeyPressOnTime(XPOWERS_POWERON_128MS);
    PMU.disableTSPinMeasure();
    PMU.enableBattDetection();
    PMU.enableVbusVoltageMeasure();
    PMU.enableBattVoltageMeasure();
    PMU.setChargingLedMode(XPOWERS_CHG_LED_OFF);

    // FIX 1: enable power-key IRQs so AXP2101 pulls INT low on button press,
    // which is the ext1 wake source.  Clear any stale status first.
    PMU.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    PMU.enableIRQ(XPOWERS_AXP2101_PKEY_SHORT_IRQ | XPOWERS_AXP2101_PKEY_LONG_IRQ);
    PMU.clearIrqStatus();

    // INT pin is driven low by the AXP2101 when an IRQ fires; external pull-up
    // keeps it high otherwise.  Configure as input (no internal pull needed).
    pinMode(PMU_INT_PIN, INPUT);

    // Backlight via LEDC (Arduino-ESP32 2.x API: ledcSetup + ledcAttachPin)
    ledcSetup(0, BL_FREQ, BL_BITS);
    ledcAttachPin(TFT_BL_PIN, 0);
    return true;
}

void powerCpuLow() { setCpuFrequencyMhz(80); }

void setBacklight(uint8_t level) {
    if (level == 0) {
        PMU.disableALDO2();            // cut panel power
    } else if (g_brightness == 0) {
        PMU.enableALDO2();             // restore panel power
        delay(5);
    }
    g_brightness = level;
    ledcWrite(0, level);  // channel 0
}

uint16_t powerBattMv()    { return PMU.getBattVoltage(); }
bool powerIsCharging()    { return PMU.isCharging(); }
void powerDisableTouchRail() { PMU.disableALDO3(); }
void powerEnableTouchRail()  { PMU.enableALDO3(); }

// FIX 2: read the IRQ status register (which deasserts the INT line) then clear
// the latch.  Must be called on every boot/wake before re-arming deep sleep.
void powerReadClearIrq() {
    PMU.getIrqStatus();
    PMU.clearIrqStatus();
}

void powerEnterDeepSleep() {
    setBacklight(0);
    powerDisableTouchRail();           // BMA rail (system 3V3) stays on for pedometer
    // FIX 2: read+clear IRQ status so INT pin goes high before we arm ext1,
    // preventing an immediate re-wake loop.
    PMU.getIrqStatus();
    PMU.clearIrqStatus();
    // Wake when the AXP2101 pulls PMU_INT low on a power-key press.
    esp_sleep_enable_ext1_wakeup(1ULL << PMU_INT_PIN, ESP_EXT1_WAKEUP_ANY_LOW);
    esp_deep_sleep_start();
}
