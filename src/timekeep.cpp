#include "timekeep.h"
#include "pins.h"
#include "config.h"
#include <WiFi.h>
#include <time.h>

// NOTE: This unit's PCF8563 does not keep time — a raw-Wire test showed writes
// to the seconds register are ACKed but never retained (reads back 0x80, VL bit
// set) and the oscillator does not advance. We therefore use the ESP32-S3
// internal RTC (system time): set by NTP, it advances in real time and is
// preserved across deep sleep by the RTC timer domain. Only a full power-off
// (dead battery) loses it, which the next NTP sync restores.
//
// The TZ environment is held in normal RAM, which is cleared across deep sleep,
// so it must be re-applied on every boot (rtcInit) before getLocalTime().

bool rtcInit() {
    setenv("TZ", TZ_STRING, 1);
    tzset();
    return true;
}

bool readClock(ClockData& out) {
    struct tm tm = {};
    if (!getLocalTime(&tm, 10)) {   // system time not set yet (cold boot pre-NTP)
        return false;
    }
    out.year    = tm.tm_year + 1900;
    out.month   = tm.tm_mon + 1;
    out.day     = tm.tm_mday;
    out.hour    = tm.tm_hour;
    out.minute  = tm.tm_min;
    out.second  = tm.tm_sec;
    out.weekday = tm.tm_wday;
    out.yearDay = tm.tm_yday;
    return out.year >= 2020;
}

bool ntpSync() {
    Serial.printf("ntp: connecting to \"%s\"...\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > WIFI_CONNECT_TIMEOUT_MS) {
            Serial.printf("ntp: WiFi connect FAILED (status=%d)\n", (int)WiFi.status());
            WiFi.disconnect(true); WiFi.mode(WIFI_OFF); return false;
        }
        delay(150);
    }
    Serial.printf("ntp: WiFi connected, ip=%s rssi=%d\n",
                  WiFi.localIP().toString().c_str(), (int)WiFi.RSSI());
    configTzTime(TZ_STRING, NTP_SERVER);   // applies TZ + starts SNTP on the ESP32 RTC
    struct tm tm = {};
    bool ok = getLocalTime(&tm, 8000);     // wait up to 8s for SNTP to set system time
    Serial.printf("ntp: SNTP %s -> %04d-%02d-%02d %02d:%02d:%02d\n",
                  ok ? "OK" : "FAILED", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                  tm.tm_hour, tm.tm_min, tm.tm_sec);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    return ok;
}
