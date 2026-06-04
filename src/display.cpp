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
