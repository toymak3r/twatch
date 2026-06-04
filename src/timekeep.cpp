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
