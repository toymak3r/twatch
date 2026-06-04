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
