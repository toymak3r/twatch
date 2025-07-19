#include <LovyanGFX.hpp>

// Display configuration for LilyGo T-Watch S3
class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;

public:
    LGFX(void)
    {
        // SPI bus configuration
        {
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.pin_sclk = 18;
            cfg.pin_mosi = 13;
            cfg.pin_miso = -1;
            cfg.pin_dc = 38;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        // Panel configuration
        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs = 12;
            cfg.pin_rst = -1;
            cfg.pin_busy = -1;
            cfg.panel_width = 240;
            cfg.panel_height = 240;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 2;
            cfg.rgb_order = false;
            cfg.invert = true;
            cfg.readable = false;
            _panel_instance.config(cfg);
        }

        // Backlight configuration
        {
            auto cfg = _light_instance.config();
            cfg.pin_bl = 45;
            cfg.invert = false;
            cfg.freq = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        setPanel(&_panel_instance);
    }
};

LGFX display;

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting T-Watch S3 with LovyanGFX...");
    
    display.init();
    display.setRotation(2);
    display.setBrightness(128);
    
    Serial.println("Display initialized!");
}

void loop()
{
    // Clear screen with different colors
    display.fillScreen(TFT_RED);
    delay(1000);
    
    display.fillScreen(TFT_GREEN);
    delay(1000);
    
    display.fillScreen(TFT_BLUE);
    delay(1000);
    
    // Show text
    display.fillScreen(TFT_BLACK);
    display.setTextColor(TFT_WHITE);
    display.setTextSize(2);
    display.setCursor(20, 100);
    display.println("T-Watch S3");
    display.setCursor(20, 130);
    display.println("LovyanGFX OK!");
    delay(2000);
} 