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
    PMU.clearIrqStatus();

    // Backlight via LEDC (Arduino-ESP32 v3 / IDF5 API)
    ledcAttach(TFT_BL_PIN, BL_FREQ, BL_BITS);
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
    ledcWrite(TFT_BL_PIN, level);
}

uint16_t powerBattMv()    { return PMU.getBattVoltage(); }
bool powerIsCharging()    { return PMU.isCharging(); }
void powerDisableTouchRail() { PMU.disableALDO3(); }
void powerEnableTouchRail()  { PMU.enableALDO3(); }

void powerEnterDeepSleep() {
    setBacklight(0);
    powerDisableTouchRail();           // BMA rail (system 3V3) stays on for pedometer
    PMU.clearIrqStatus();
    // Wake when the AXP2101 pulls PMU_INT low on a power-key press.
    esp_sleep_enable_ext1_wakeup(1ULL << PMU_INT_PIN, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_deep_sleep_start();
}
