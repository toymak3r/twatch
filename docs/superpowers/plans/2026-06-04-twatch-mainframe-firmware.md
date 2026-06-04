# MAINFRAME Deep-Sleep Clock — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a battery-optimized T-Watch S3 firmware that wakes on the physical button, shows a CRT "MAINFRAME" clock for 10s, then deep-sleeps — targeting days of battery life.

**Architecture:** Deep-sleep-centric. Each button press boots the ESP32-S3 from deep sleep; `setup()` does minimal init + renders, `loop()` handles 10s of touch gestures then re-enters deep sleep (wake = AXP2101 power button on GPIO21). No LVGL, no `LilyGoLib` wrapper — direct LovyanGFX drawing + XPowersLib (AXP2101) + SensorLib (PCF8563 RTC, BMA423 pedometer). Pure logic (battery %, NTP-sync decision, gesture classification) is isolated and unit-tested on host.

**Tech Stack:** PlatformIO / Arduino-ESP32, LovyanGFX, XPowersLib, SensorLib, ESP32 WiFi+SNTP, `esp_deep_sleep`. Spec: `docs/superpowers/specs/2026-06-04-twatch-mainframe-clock-design.md`.

---

## File Structure

All firmware lives in `src/`. Old `LilyGoLib`/LVGL files are removed in Task 1.

| File | Responsibility |
|---|---|
| `src/pins.h` | T-Watch S3 GPIO map (ST7789, touch, I2C, PMU button) |
| `src/config.h` | User config: WiFi creds, timezone, NTP server, brightness, timeout, feature toggles |
| `src/types.h` | Shared enums/structs: `Page`, `Gesture`, `ClockData`, `DeviceState`, persistent RTC state |
| `src/battery.h` | Pure `batteryPercent(mv)` (header-only, host-testable) |
| `src/sync_policy.h` | Pure `needsNtpSync(...)` (header-only, host-testable) |
| `src/gesture.h` | Pure `classifySwipe(dx,dy)` + `isDoubleTap(...)` (header-only, host-testable) |
| `src/power.h` / `power.cpp` | AXP2101 init, backlight, display sleep/wake, CPU freq, deep sleep |
| `src/display.h` / `display.cpp` | LovyanGFX `LGFX` device + `displayInit()` + page renderers |
| `src/timekeep.h` / `timekeep.cpp` | PCF8563 read + WiFi/SNTP sync |
| `src/input.h` / `input.cpp` | FT6X36 init + `pollGesture()` |
| `src/pedometer.h` / `pedometer.cpp` | BMA423 step counter (persists through deep sleep) |
| `src/main.cpp` | Orchestration: wake → init → render → 10s loop → deep sleep |
| `test/test_battery/` `test_sync/` `test_gesture/` | Native (host) unit tests for pure modules |

---

## Task 1: Reset build system to a consistent, minimal stack

**Files:**
- Modify: `platformio.ini` (full rewrite)
- Delete: `src/LilyGoLib.cpp`, `src/LilyGoLib.h`, `src/LilyGoLib_Warning.h`, `src/LV_Helper.cpp`, `src/LV_Helper.h`, `src/lv_conf.h`, `src/build_opt.h`, `src/t-watch.ino`
- Keep: nothing else in `src/` yet (we recreate from scratch)

- [ ] **Step 1: Extract the AXP2101 rail values for reference**

Before deleting, note the proven values from `src/LilyGoLib.cpp::beginPower()` (already captured in this plan's Task 6). No action beyond confirming Task 6 matches.

- [ ] **Step 2: Delete the old wrapper/LVGL files**

```bash
git rm src/LilyGoLib.cpp src/LilyGoLib.h src/LilyGoLib_Warning.h \
       src/LV_Helper.cpp src/LV_Helper.h src/lv_conf.h src/build_opt.h src/t-watch.ino
```

- [ ] **Step 3: Rewrite `platformio.ini`**

```ini
; T-Watch S3 — MAINFRAME deep-sleep clock
[env:twatch-s3]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino

build_flags =
    -Os
    -DCORE_DEBUG_LEVEL=0
    -DBOARD_HAS_PSRAM
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DLGFX_USE_V1

lib_deps =
    lovyan03/LovyanGFX@^1.1.16
    lewisxhe/XPowersLib@^0.2.4
    lewisxhe/SensorLib@^0.1.8

board_build.partitions = partitions_3M_9M.csv
board_build.flash_mode = qio
monitor_speed = 115200
monitor_filters = esp32_exception_decoder, default
upload_speed = 921600

; Host-side unit tests for pure logic (no hardware)
[env:native]
platform = native
test_framework = unity
build_flags = -std=gnu++17
```

- [ ] **Step 4: Verify the project no longer references deleted files**

Run: `git status` — expect the 8 files staged as deleted and `platformio.ini` modified.

- [ ] **Step 5: Commit**

```bash
git add platformio.ini
git commit -m "build: reset to minimal LovyanGFX+XPowersLib+SensorLib stack"
```

---

## Task 2: Pin map and config

**Files:**
- Create: `src/pins.h`
- Create: `src/config.h`

- [ ] **Step 1: Create `src/pins.h`** (values from the proven S3 block of `utilities.h`)

```cpp
#pragma once
// LilyGo T-Watch S3 GPIO map (ESP32-S3)
// Display ST7789 (SPI)
#define TFT_MOSI_PIN   13
#define TFT_SCLK_PIN   18
#define TFT_CS_PIN     12
#define TFT_DC_PIN     38
#define TFT_RST_PIN    -1
#define TFT_BL_PIN     45
#define TFT_W          240
#define TFT_H          240
// Touch FT6X36 (I2C bus 1)
#define TOUCH_SDA_PIN  39
#define TOUCH_SCL_PIN  40
#define TOUCH_INT_PIN  16
// Main I2C bus 0 (AXP2101, PCF8563, BMA423, DRV2605)
#define I2C_SDA_PIN    10
#define I2C_SCL_PIN    11
// AXP2101 power-button interrupt (deep-sleep wake source, RTC-capable)
#define PMU_INT_PIN    21
// Backlight LEDC
#define BL_FREQ        1000
#define BL_BITS        8
```

- [ ] **Step 2: Create `src/config.h`**

```cpp
#pragma once
// ---- USER CONFIG: fill these in ----
#define WIFI_SSID      "YOUR_WIFI_SSID"
#define WIFI_PASS      "YOUR_WIFI_PASSWORD"
// POSIX TZ for America/Sao_Paulo (UTC-3, no DST):
#define TZ_STRING      "<-03>3"
#define NTP_SERVER     "pool.ntp.org"
// ---- Behaviour ----
#define AWAKE_BRIGHTNESS   80      // 0..255 backlight when screen is on
#define SCREEN_TIMEOUT_MS  10000   // auto-sleep after this idle time
#define PEDOMETER_ENABLED  1       // keep BMA423 step counter alive in sleep
#define WIFI_CONNECT_TIMEOUT_MS 8000
#define LOW_BATTERY_MV     3300    // below this: warn + sleep immediately
```

- [ ] **Step 3: Commit**

```bash
git add src/pins.h src/config.h
git commit -m "feat: add T-Watch S3 pin map and user config"
```

---

## Task 3: Pure module — battery percentage (TDD)

**Files:**
- Create: `src/battery.h`
- Create: `test/test_battery/test_battery.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
// test/test_battery/test_battery.cpp
#include <unity.h>
#include "../../src/battery.h"

void test_full_above_max(void)  { TEST_ASSERT_EQUAL_UINT8(100, batteryPercent(4250)); }
void test_empty_below_min(void) { TEST_ASSERT_EQUAL_UINT8(0,   batteryPercent(3200)); }
void test_midpoint(void)        { TEST_ASSERT_EQUAL_UINT8(50,  batteryPercent(3750)); }
void test_quarter(void)         { TEST_ASSERT_EQUAL_UINT8(25,  batteryPercent(3525)); }

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_full_above_max);
    RUN_TEST(test_empty_below_min);
    RUN_TEST(test_midpoint);
    RUN_TEST(test_quarter);
    return UNITY_END();
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `pio test -e native -f test_battery`
Expected: FAIL — `battery.h` not found / `batteryPercent` undefined.

- [ ] **Step 3: Write minimal implementation**

```cpp
// src/battery.h
#pragma once
#include <stdint.h>
// Li-ion 4200mV=100% .. 3300mV=0%, clamped, linear.
inline uint8_t batteryPercent(uint16_t mv) {
    if (mv >= 4200) return 100;
    if (mv <= 3300) return 0;
    return (uint8_t)(((uint32_t)(mv - 3300) * 100) / 900);
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `pio test -e native -f test_battery`
Expected: PASS (4 tests).

- [ ] **Step 5: Commit**

```bash
git add src/battery.h test/test_battery/test_battery.cpp
git commit -m "feat: add tested batteryPercent pure function"
```

---

## Task 4: Pure module — NTP sync policy (TDD)

**Files:**
- Create: `src/sync_policy.h`
- Create: `test/test_sync/test_sync.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
// test/test_sync/test_sync.cpp
#include <unity.h>
#include "../../src/sync_policy.h"

void test_never_synced(void)      { TEST_ASSERT_TRUE(needsNtpSync(false, 0, 155)); }
void test_same_day_no_sync(void)  { TEST_ASSERT_FALSE(needsNtpSync(true, 155, 155)); }
void test_new_day_sync(void)      { TEST_ASSERT_TRUE(needsNtpSync(true, 155, 156)); }

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_never_synced);
    RUN_TEST(test_same_day_no_sync);
    RUN_TEST(test_new_day_sync);
    return UNITY_END();
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `pio test -e native -f test_sync`
Expected: FAIL — `sync_policy.h` not found.

- [ ] **Step 3: Write minimal implementation**

```cpp
// src/sync_policy.h
#pragma once
// Sync if we've never synced, or the calendar day changed since last sync.
// yearDay = day-of-year (0..365) read from the RTC.
inline bool needsNtpSync(bool everSynced, int lastSyncYearDay, int currentYearDay) {
    if (!everSynced) return true;
    return lastSyncYearDay != currentYearDay;
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `pio test -e native -f test_sync`
Expected: PASS (3 tests).

- [ ] **Step 5: Commit**

```bash
git add src/sync_policy.h test/test_sync/test_sync.cpp
git commit -m "feat: add tested needsNtpSync pure function"
```

---

## Task 5: Pure module — gesture classification (TDD)

**Files:**
- Create: `src/gesture.h`
- Create: `test/test_gesture/test_gesture.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
// test/test_gesture/test_gesture.cpp
#include <unity.h>
#include "../../src/gesture.h"

void test_swipe_up(void)    { TEST_ASSERT_EQUAL_INT(GESTURE_SWIPE_UP,   classifySwipe(5, -60)); }
void test_swipe_down(void)  { TEST_ASSERT_EQUAL_INT(GESTURE_SWIPE_DOWN, classifySwipe(-8, 70)); }
void test_too_small(void)   { TEST_ASSERT_EQUAL_INT(GESTURE_NONE,       classifySwipe(5, 10)); }
void test_horizontal(void)  { TEST_ASSERT_EQUAL_INT(GESTURE_NONE,       classifySwipe(80, 5)); }
void test_double_tap_yes(void){ TEST_ASSERT_TRUE(isDoubleTap(1000, 1250)); }
void test_double_tap_no(void) { TEST_ASSERT_FALSE(isDoubleTap(1000, 1600)); }

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_swipe_up);
    RUN_TEST(test_swipe_down);
    RUN_TEST(test_too_small);
    RUN_TEST(test_horizontal);
    RUN_TEST(test_double_tap_yes);
    RUN_TEST(test_double_tap_no);
    return UNITY_END();
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `pio test -e native -f test_gesture`
Expected: FAIL — `gesture.h` not found.

- [ ] **Step 3: Write minimal implementation**

```cpp
// src/gesture.h
#pragma once
#include <stdint.h>
#include <stdlib.h>
enum Gesture { GESTURE_NONE, GESTURE_SWIPE_UP, GESTURE_SWIPE_DOWN, GESTURE_DOUBLE_TAP };
#define SWIPE_MIN_PX        40
#define DOUBLE_TAP_MAX_MS   400
// Vertical swipe only: dominant vertical movement over threshold.
inline Gesture classifySwipe(int dx, int dy) {
    if (abs(dy) > abs(dx) && abs(dy) >= SWIPE_MIN_PX)
        return dy < 0 ? GESTURE_SWIPE_UP : GESTURE_SWIPE_DOWN;
    return GESTURE_NONE;
}
inline bool isDoubleTap(uint32_t prevTapMs, uint32_t nowMs) {
    return (nowMs - prevTapMs) <= DOUBLE_TAP_MAX_MS;
}
```

- [ ] **Step 4: Run test to verify it passes**

Run: `pio test -e native -f test_gesture`
Expected: PASS (6 tests).

- [ ] **Step 5: Commit**

```bash
git add src/gesture.h test/test_gesture/test_gesture.cpp
git commit -m "feat: add tested gesture classification pure functions"
```

---

## Task 6: Shared types + power module (AXP2101, backlight, deep sleep)

**Files:**
- Create: `src/types.h`
- Create: `src/power.h`, `src/power.cpp`

- [ ] **Step 1: Create `src/types.h`**

```cpp
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
```

- [ ] **Step 2: Create `src/power.h`**

```cpp
#pragma once
#include <stdint.h>
bool powerInit();                 // AXP2101 rails + button config; false if PMU missing
void powerCpuLow();               // 80 MHz
void setBacklight(uint8_t level); // 0 = panel+backlight off
uint16_t powerBattMv();
bool powerIsCharging();
void powerDisableTouchRail();     // ALDO3 off (for sleep)
void powerEnableTouchRail();      // ALDO3 on (on wake)
void powerEnterDeepSleep();       // wake = power button (GPIO21)
extern uint8_t g_brightness;
```

- [ ] **Step 3: Create `src/power.cpp`** (rail values verbatim from proven `beginPower()`; radio rail ALDO4 left OFF since unused)

```cpp
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
```

- [ ] **Step 4: Build to verify it compiles (no link yet — needs main; defer full build to Task 11). Sanity-check headers compile via a throwaway build is optional.**

Run: `pio run -e twatch-s3` (will fail at link until `main.cpp` exists — that's expected; confirm `power.cpp` has no *compile* errors in the output).

- [ ] **Step 5: Commit**

```bash
git add src/types.h src/power.h src/power.cpp
git commit -m "feat: add AXP2101 power module (rails, backlight, deep sleep)"
```

---

## Task 7: Display module — LovyanGFX device + init

**Files:**
- Create: `src/display.h`, `src/display.cpp`

- [ ] **Step 1: Create `src/display.h`**

```cpp
#pragma once
#include "types.h"
void displayInit();
void renderClockPage(const ClockData& t, const DeviceState& d);
void renderSystemPage(const ClockData& t, const DeviceState& d);
void renderDatePage(const ClockData& t, const DeviceState& d);
void renderLowBattery(uint16_t mv);
```

- [ ] **Step 2: Create the LovyanGFX device + init in `src/display.cpp`** (ST7789 240x240 over SPI; pins from `pins.h`)

```cpp
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include "display.h"
#include "pins.h"

class LGFX : public lgfx::LGFX_Device {
    lgfx::Panel_ST7789  _panel;
    lgfx::Bus_SPI       _bus;
public:
    LGFX() {
        { auto c = _bus.config();
          c.spi_host = SPI2_HOST; c.spi_mode = 0;
          c.freq_write = 40000000; c.freq_read = 16000000;
          c.pin_sclk = TFT_SCLK_PIN; c.pin_mosi = TFT_MOSI_PIN;
          c.pin_miso = -1; c.pin_dc = TFT_DC_PIN;
          _bus.config(c); _panel.setBus(&_bus); }
        { auto c = _panel.config();
          c.pin_cs = TFT_CS_PIN; c.pin_rst = TFT_RST_PIN;
          c.panel_width = TFT_W; c.panel_height = TFT_H;
          c.offset_x = 0; c.offset_y = 0;
          c.invert = true; c.rgb_order = false;
          _panel.config(c); }
        setPanel(&_panel);
    }
};
static LGFX tft;

// MAINFRAME palette
static constexpr uint32_t COL_PHOSPHOR = 0x14FF5E; // green
static constexpr uint32_t COL_TIME     = 0xFF2222; // red
static constexpr uint32_t COL_SEC      = 0xFF8A00; // orange
static constexpr uint32_t COL_DIM      = 0x0A6A2A;

void displayInit() {
    tft.init();
    tft.setRotation(2);
    tft.fillScreen(TFT_BLACK);
}
```

- [ ] **Step 3: Build to confirm the LovyanGFX device compiles**

Run: `pio run -e twatch-s3`
Expected: compiles `display.cpp` (link still fails until `main.cpp`; confirm no display.cpp errors).

- [ ] **Step 4: Commit**

```bash
git add src/display.h src/display.cpp
git commit -m "feat: add LovyanGFX ST7789 device and display init"
```

---

## Task 8: Display module — the three page renderers

**Files:**
- Modify: `src/display.cpp` (append renderers)

- [ ] **Step 1: Append the renderers to `src/display.cpp`**

```cpp
static const char* WD[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
static const char* MO[] = {"JAN","FEB","MAR","APR","MAY","JUN",
                           "JUL","AUG","SEP","OCT","NOV","DEC"};

static void drawHeader(const DeviceState& d) {
    tft.setTextColor(COL_PHOSPHOR, TFT_BLACK);
    tft.setTextDatum(textdatum_t::top_left);
    tft.setFont(&fonts::Font2);
    tft.drawString(d.syncOk ? "WIFI:OK" : "WIFI:--", 14, 12);
    tft.setTextDatum(textdatum_t::top_right);
    char b[16]; snprintf(b, sizeof(b), "%s%d%%", d.charging ? "CHG " : "BAT ", d.battPct);
    tft.drawString(b, TFT_W - 14, 12);
}

void renderClockPage(const ClockData& t, const DeviceState& d) {
    tft.fillScreen(TFT_BLACK);
    drawHeader(d);
    // Big red HH:MM centered
    tft.setTextColor(COL_TIME, TFT_BLACK);
    tft.setTextDatum(textdatum_t::middle_center);
    tft.setFont(&fonts::Font7);            // 7-seg style numerals
    char hm[8]; snprintf(hm, sizeof(hm), "%02d:%02d", t.hour, t.minute);
    tft.drawString(hm, TFT_W/2, 104);
    // small orange seconds
    tft.setTextColor(COL_SEC, TFT_BLACK);
    tft.setFont(&fonts::Font4);
    char ss[4]; snprintf(ss, sizeof(ss), ":%02d", t.second);
    tft.setTextDatum(textdatum_t::middle_left);
    tft.drawString(ss, TFT_W/2 + 78, 104);
    // date line
    tft.setTextColor(COL_PHOSPHOR, TFT_BLACK);
    tft.setTextDatum(textdatum_t::middle_center);
    tft.setFont(&fonts::Font2);
    char dl[24]; snprintf(dl, sizeof(dl), "// %s %04d-%02d-%02d",
                          WD[t.weekday % 7], t.year, t.month, t.day);
    tft.drawString(dl, TFT_W/2, 150);
    // power bar [||||····]
    int filled = (d.battPct * 12) / 100;
    char bar[20] = "["; for (int i=0;i<12;i++) strcat(bar, i<filled? "|":".");
    strcat(bar, "] PWR");
    tft.drawString(bar, TFT_W/2, 210);
}

void renderSystemPage(const ClockData& t, const DeviceState& d) {
    tft.fillScreen(TFT_BLACK);
    drawHeader(d);
    tft.setTextColor(COL_PHOSPHOR, TFT_BLACK);
    tft.setTextDatum(textdatum_t::top_left);
    tft.setFont(&fonts::Font4);
    char l[32];
    snprintf(l, sizeof(l), "PWR  %d%%  %dmV", d.battPct, d.battMv); tft.drawString(l, 20, 56);
    snprintf(l, sizeof(l), "CHG  %s", d.charging ? "YES" : "no");   tft.drawString(l, 20, 96);
    snprintf(l, sizeof(l), "TEMP %.1f C", d.coreTemp);              tft.drawString(l, 20, 136);
    if (d.syncOk) snprintf(l, sizeof(l), "SYNC %dd ago", d.syncAgeDays);
    else          snprintf(l, sizeof(l), "SYNC never");
    tft.drawString(l, 20, 176);
}

void renderDatePage(const ClockData& t, const DeviceState& d) {
    tft.fillScreen(TFT_BLACK);
    drawHeader(d);
    tft.setTextColor(COL_PHOSPHOR, TFT_BLACK);
    tft.setTextDatum(textdatum_t::middle_center);
    tft.setFont(&fonts::Font4);
    tft.drawString(WD[t.weekday % 7], TFT_W/2, 70);
    char dl[20]; snprintf(dl, sizeof(dl), "%02d %s %04d", t.day, MO[(t.month-1)%12], t.year);
    tft.drawString(dl, TFT_W/2, 110);
    char st[24]; snprintf(st, sizeof(st), "STEPS %lu", (unsigned long)d.steps);
    tft.setTextColor(COL_SEC, TFT_BLACK);
    tft.drawString(st, TFT_W/2, 160);
}

void renderLowBattery(uint16_t mv) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(COL_TIME, TFT_BLACK);
    tft.setTextDatum(textdatum_t::middle_center);
    tft.setFont(&fonts::Font4);
    char l[24]; snprintf(l, sizeof(l), "LOW BATT %dmV", mv);
    tft.drawString(l, TFT_W/2, TFT_H/2);
}
```

- [ ] **Step 2: Build to confirm renderers compile**

Run: `pio run -e twatch-s3`
Expected: `display.cpp` compiles clean (link fails until main — OK).

- [ ] **Step 3: Commit**

```bash
git add src/display.cpp
git commit -m "feat: add MAINFRAME page renderers (clock/system/date/low-batt)"
```

---

## Task 9: Timekeeping module (PCF8563 + WiFi/SNTP)

**Files:**
- Create: `src/timekeep.h`, `src/timekeep.cpp`

- [ ] **Step 1: Create `src/timekeep.h`**

```cpp
#pragma once
#include "types.h"
bool rtcInit();              // PCF8563 over Wire (I2C bus 0)
bool readClock(ClockData& out);
bool ntpSync();              // WiFi up -> SNTP -> write PCF8563 -> WiFi off; true if set
```

- [ ] **Step 2: Create `src/timekeep.cpp`**

```cpp
#include "timekeep.h"
#include "pins.h"
#include "config.h"
#include <Wire.h>
#include <SensorPCF8563.hpp>
#include <WiFi.h>
#include <time.h>

static SensorPCF8563 rtc;

bool rtcInit() {
    if (!rtc.init(Wire, I2C_SDA_PIN, I2C_SCL_PIN)) return false;
    rtc.disableCLK();   // save backup-battery current
    return true;
}

bool readClock(ClockData& out) {
    struct tm tm = rtc.getDateTime();
    out.year = tm.tm_year + 1900; out.month = tm.tm_mon + 1; out.day = tm.tm_mday;
    out.hour = tm.tm_hour; out.minute = tm.tm_min; out.second = tm.tm_sec;
    out.weekday = tm.tm_wday; out.yearDay = tm.tm_yday;
    return out.year >= 2020;   // false => RTC not set yet
}

bool ntpSync() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > WIFI_CONNECT_TIMEOUT_MS) { WiFi.disconnect(true); WiFi.mode(WIFI_OFF); return false; }
        delay(150);
    }
    configTzTime(TZ_STRING, NTP_SERVER);
    struct tm tm;
    bool ok = getLocalTime(&tm, 6000);   // wait up to 6s for SNTP
    if (ok) rtc.setDateTime(tm);         // persist to PCF8563
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return ok;
}
```

- [ ] **Step 3: Build to confirm compile**

Run: `pio run -e twatch-s3`
Expected: `timekeep.cpp` compiles clean. (If `getDateTime`/`setDateTime` signatures differ in the installed SensorLib version, adapt to the version's `RTC_DateTime` API — see `~/.platformio/lib*/SensorLib*/examples/PCF8563`.)

- [ ] **Step 4: Commit**

```bash
git add src/timekeep.h src/timekeep.cpp
git commit -m "feat: add PCF8563 timekeeping with WiFi/NTP sync"
```

---

## Task 10: Input (FT6X36 gestures) + pedometer (BMA423)

**Files:**
- Create: `src/input.h`, `src/input.cpp`, `src/pedometer.h`, `src/pedometer.cpp`

- [ ] **Step 1: Create `src/input.h`**

```cpp
#pragma once
#include "gesture.h"
bool touchInit();
Gesture pollGesture();   // call repeatedly while awake; returns a gesture or GESTURE_NONE
```

- [ ] **Step 2: Create `src/input.cpp`** (swipe from touch-down→touch-up deltas; double-tap from tap timing)

```cpp
#include "input.h"
#include "pins.h"
#include <Wire.h>
#include <TouchDrvFT6X36.hpp>

static TouchDrvFT6X36 touch;
static bool wasDown = false;
static int16_t downX = 0, downY = 0;
static uint32_t lastTapMs = 0;

bool touchInit() {
    Wire1.begin(TOUCH_SDA_PIN, TOUCH_SCL_PIN);
    pinMode(TOUCH_INT_PIN, INPUT);
    return touch.begin(Wire1, FT6X36_SLAVE_ADDRESS, TOUCH_SDA_PIN, TOUCH_SCL_PIN);
}

Gesture pollGesture() {
    int16_t x[1], y[1];
    bool down = touch.getPoint(x, y, 1) > 0;
    if (down && !wasDown) { wasDown = true; downX = x[0]; downY = y[0]; return GESTURE_NONE; }
    if (!down && wasDown) {                       // release: decide swipe vs tap
        wasDown = false;
        int dx = (int)x[0] - downX, dy = (int)y[0] - downY;  // last known point
        Gesture g = classifySwipe(dx, dy);
        if (g != GESTURE_NONE) return g;
        uint32_t now = millis();                  // tap
        if (isDoubleTap(lastTapMs, now)) { lastTapMs = 0; return GESTURE_DOUBLE_TAP; }
        lastTapMs = now;
    }
    return GESTURE_NONE;
}
```

- [ ] **Step 3: Create `src/pedometer.h`**

```cpp
#pragma once
#include <stdint.h>
bool pedoInit();        // BMA423 step counter; stays powered through deep sleep
uint32_t getSteps();
```

- [ ] **Step 4: Create `src/pedometer.cpp`**

```cpp
#include "pedometer.h"
#include "pins.h"
#include "config.h"
#include <Wire.h>
#include <SensorBMA423.hpp>

static SensorBMA423 accel;
static bool ok = false;

bool pedoInit() {
#if PEDOMETER_ENABLED
    ok = accel.init(Wire, I2C_SDA_PIN, I2C_SCL_PIN);
    if (ok) {
        accel.configAccelerometer();
        accel.enableAccelerometer();
        accel.enablePedometer();          // hardware step counter (low power, persists while powered)
        accel.enableFeature(SensorBMA423::FEATURE_STEP_CNTR, true);
    }
    return ok;
#else
    return false;
#endif
}

uint32_t getSteps() { return ok ? accel.getPedometerCounter() : 0; }
```

- [ ] **Step 5: Build to confirm compile**

Run: `pio run -e twatch-s3`
Expected: `input.cpp` and `pedometer.cpp` compile clean. (Adapt `getPoint` / BMA feature calls to the installed SensorLib version if signatures differ — check `SensorLib/examples`.)

- [ ] **Step 6: Commit**

```bash
git add src/input.h src/input.cpp src/pedometer.h src/pedometer.cpp
git commit -m "feat: add FT6X36 gesture input and BMA423 pedometer"
```

---

## Task 11: Orchestration — `main.cpp` (wake → render → 10s → deep sleep)

**Files:**
- Create: `src/main.cpp`

- [ ] **Step 1: Create `src/main.cpp`**

```cpp
#include <Arduino.h>
#include "config.h"
#include "types.h"
#include "battery.h"
#include "sync_policy.h"
#include "power.h"
#include "display.h"
#include "timekeep.h"
#include "input.h"
#include "pedometer.h"

RTC_DATA_ATTR uint32_t bootCount    = 0;
RTC_DATA_ATTR bool     everSynced   = false;
RTC_DATA_ATTR int      lastSyncYDay = -1;
RTC_DATA_ATTR bool     lastSyncOk   = false;

static Page page = PAGE_CLOCK;
static uint32_t lastInteraction = 0;

static void gatherState(ClockData& t, DeviceState& d) {
    readClock(t);
    d.battMv = powerBattMv();
    d.battPct = batteryPercent(d.battMv);
    d.charging = powerIsCharging();
    d.coreTemp = temperatureRead();
    d.steps = getSteps();
    d.syncOk = lastSyncOk;
    d.syncAgeDays = (everSynced && t.yearDay >= lastSyncYDay) ? (t.yearDay - lastSyncYDay) : 0;
}

static void renderCurrent(const ClockData& t, const DeviceState& d) {
    switch (page) {
        case PAGE_CLOCK:  renderClockPage(t, d);  break;
        case PAGE_SYSTEM: renderSystemPage(t, d); break;
        case PAGE_DATE:   renderDatePage(t, d);   break;
        default: break;
    }
}

void setup() {
    Serial.begin(115200);
    bootCount++;
    if (!powerInit()) { Serial.println("PMU init failed"); }
    powerCpuLow();

    uint16_t mv = powerBattMv();
    if (mv > 0 && mv < LOW_BATTERY_MV && !powerIsCharging()) {
        displayInit(); setBacklight(AWAKE_BRIGHTNESS);
        renderLowBattery(mv); delay(1500);
        powerEnterDeepSleep();           // does not return
    }

    rtcInit();
    pedoInit();

    ClockData t{}; readClock(t);
    if (needsNtpSync(everSynced, lastSyncYDay, t.yearDay)) {
        bool ok = ntpSync();
        lastSyncOk = ok;
        if (ok) { everSynced = true; readClock(t); lastSyncYDay = t.yearDay; }
    }

    displayInit();
    powerEnableTouchRail();
    touchInit();
    setBacklight(AWAKE_BRIGHTNESS);

    page = PAGE_CLOCK;
    DeviceState d{}; gatherState(t, d);
    renderCurrent(t, d);
    lastInteraction = millis();
    Serial.printf("wake#%lu batt=%dmV sync=%d\n", (unsigned long)bootCount, mv, lastSyncOk);
}

void loop() {
    static uint32_t lastTick = 0;
    Gesture g = pollGesture();
    if (g != GESTURE_NONE) {
        lastInteraction = millis();
        ClockData t{}; DeviceState d{}; gatherState(t, d);
        if (g == GESTURE_SWIPE_UP)        page = (Page)((page + 1) % PAGE_COUNT);
        else if (g == GESTURE_SWIPE_DOWN) page = (Page)((page + PAGE_COUNT - 1) % PAGE_COUNT);
        else if (g == GESTURE_DOUBLE_TAP) {
            bool ok = ntpSync(); lastSyncOk = ok;
            if (ok) { everSynced = true; ClockData nt{}; readClock(nt); lastSyncYDay = nt.yearDay; }
            gatherState(t, d);
        }
        renderCurrent(t, d);
    }
    if (page == PAGE_CLOCK && millis() - lastTick > 1000) {     // tick seconds
        lastTick = millis();
        ClockData t{}; DeviceState d{}; gatherState(t, d);
        renderClockPage(t, d);
    }
    if (millis() - lastInteraction > SCREEN_TIMEOUT_MS) {
        Serial.println("sleep");
        powerEnterDeepSleep();           // does not return
    }
    delay(20);
}
```

- [ ] **Step 2: Full build + link**

Run: `pio run -e twatch-s3`
Expected: builds and links successfully (firmware .bin produced).

- [ ] **Step 3: Commit**

```bash
git add src/main.cpp
git commit -m "feat: orchestrate wake/render/gesture/deep-sleep flow"
```

---

## Task 12: On-device bring-up and battery validation

**Files:** none (hardware verification + notes)

- [ ] **Step 1: Fill WiFi creds**

Edit `src/config.h`: set real `WIFI_SSID` / `WIFI_PASS`. (Do NOT commit real creds — see Step 6.)

- [ ] **Step 2: Flash**

Run: `pio run -e twatch-s3 -t upload --upload-port COM5`
Expected: upload succeeds; watch reboots.

- [ ] **Step 3: Verify wake/sleep on device (serial)**

Run: `pio device monitor -b 115200`
Expected: on button press → `wake#N ... sync=...`; after 10s idle → `sleep`; screen goes dark; next button press wakes again. Verify swipe ↑/↓ changes pages and double-tap triggers a sync line.

- [ ] **Step 4: Verify NTP + clock persistence**

After first synced wake, unplug USB, wait, press button: time is correct and advances (PCF8563 kept time through deep sleep). Verify the WIFI:OK indicator.

- [ ] **Step 5: Battery validation (on battery, not USB)**

Charge full, unplug, record `battPct` from the SYSTEM page. Use normally (button presses) for several hours; confirm drain is consistent with the multi-day estimate (≈ ≤2% per hour idle). Log any anomaly (e.g., stuck-awake, high standby) and return to `superpowers:systematic-debugging`.

- [ ] **Step 6: Protect credentials**

```bash
git update-index --skip-worktree src/config.h   # keep local creds out of commits
```
Expected: future `git status` ignores local `config.h` edits. (Alternative: add a `config.example.h` and gitignore `config.h` — choose during execution.)

---

## Task 13: Documentation reconciliation (deferred from cleanup)

**Files:**
- Modify/Delete: docs that reference removed examples (see cleanup notes / `git grep` in Step 1)

- [ ] **Step 1: Re-list stale references**

Run: `git grep -n -i -E "examples/|MaxBatteryLife|cp examples" -- '*.md' '*.MD'`

- [ ] **Step 2: Rewrite the primary docs to match the new firmware**

Update `README.md`, `docs/index.md`, `docs/guides/Quick_Start_Guide.md`, `docs/BATTERY_OPTIMIZATION.md` to describe: the single `src/` firmware, `pio run -e twatch-s3 -t upload`, the deep-sleep/button model, and `config.h`. Remove the "copy an example into src/" workflow.

- [ ] **Step 3: Resolve the duplicate README**

Decide between `README.MD` and `README.md` (keep one; `git rm` the other). Fold in the pending uncommitted `README.MD` change if still wanted.

- [ ] **Step 4: Delete docs describing non-existent firmware**

`git rm docs/examples/weather-wifi-watch.md docs/examples/ble-notifications.md docs/examples/android-app-guide.md` (or rewrite if any are still desired).

- [ ] **Step 5: Commit**

```bash
git add -A
git commit -m "docs: reconcile documentation with MAINFRAME firmware"
```

---

## Self-Review notes

- **Spec coverage:** stack (T1), config/pins (T2), pure battery%/sync/gesture (T3-5), AXP2101 power + deep sleep + button wake (T6), display + 3 pages (T7-8), PCF8563 + NTP policy (T9), gestures + pedometer (T10), orchestration with 10s timeout + low-battery guard + RTC_DATA_ATTR state (T11), battery validation (T12), deferred doc cleanup (T13). All spec sections mapped.
- **Hardware-API caveat:** SensorLib (`getDateTime`/`setDateTime`, `getPoint`, BMA feature flags) and LovyanGFX panel flags (`invert`/`offset`) are the two spots most likely to need a one-line adjustment to match the exact installed library version; each such step says so and points to the library's `examples/`. These are bring-up adjustments, not design gaps.
- **Type consistency:** `Page`, `Gesture`, `ClockData`, `DeviceState` defined in `types.h`/`gesture.h` and used consistently; `g_brightness` shared via `power.h`; renderer names match between `display.h` and `main.cpp`.
