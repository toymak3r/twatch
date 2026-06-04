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
