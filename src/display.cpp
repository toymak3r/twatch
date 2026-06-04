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
