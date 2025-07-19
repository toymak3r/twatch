#include <LovyanGFX.hpp>
#include <Wire.h>

// Board pin definitions (from Setup212_LilyGo_T_Watch_S3.h)
#define BOARD_LCD_SCLK    18  // TFT_SCLK
#define BOARD_LCD_MOSI    13  // TFT_MOSI
#define BOARD_LCD_MISO    -1  // TFT_MISO (Not connected)
#define BOARD_LCD_DC      38  // TFT_DC
#define BOARD_LCD_CS      12  // TFT_CS
#define BOARD_LCD_RST     -1  // TFT_RST (Not connected)
#define BOARD_LCD_BL      45  // TFT_BL (LED back-light)

#define BOARD_TOUCH_SDA   21
#define BOARD_TOUCH_SCL   22
#define BOARD_TOUCH_INT   16

// LovyanGFX Display Configuration
class LGFX : public lgfx::LGFX_Device
{
    lgfx::Panel_ST7789 _panel_instance;
    lgfx::Bus_SPI _bus_instance;
    lgfx::Light_PWM _light_instance;
    lgfx::Touch_FT5x06 _touch_instance;

public:
    LGFX(void)
    {
        { // Bus configuration
            auto cfg = _bus_instance.config();
            cfg.spi_host = SPI2_HOST;
            cfg.spi_mode = 0;
            cfg.freq_write = 40000000;
            cfg.pin_sclk = BOARD_LCD_SCLK;
            cfg.pin_mosi = BOARD_LCD_MOSI;
            cfg.pin_miso = BOARD_LCD_MISO;
            cfg.pin_dc = BOARD_LCD_DC;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        { // Panel configuration
            auto cfg = _panel_instance.config();
            cfg.pin_cs = BOARD_LCD_CS;
            cfg.pin_rst = BOARD_LCD_RST;
            cfg.pin_busy = -1;
            cfg.panel_width = 240;
            cfg.panel_height = 240;
            cfg.offset_x = 0;
            cfg.offset_y = 0;
            cfg.offset_rotation = 0;
            cfg.rgb_order = false;
            cfg.invert = true;  // ST7789 needs inversion
            cfg.readable = false;
            _panel_instance.config(cfg);
        }

        { // Light configuration
            auto cfg = _light_instance.config();
            cfg.pin_bl = BOARD_LCD_BL;
            cfg.invert = false;
            cfg.freq = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }

        { // Touch configuration
            auto cfg = _touch_instance.config();
            cfg.pin_sda = BOARD_TOUCH_SDA;
            cfg.pin_scl = BOARD_TOUCH_SCL;
            cfg.pin_int = BOARD_TOUCH_INT;
            cfg.i2c_addr = 0x38;
            cfg.freq = 2500000;
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }

        setPanel(&_panel_instance);
    }
};

LGFX display;

// Colors (adjusted for ST7789)
#define COLOR_BACKGROUND      0xFFFF00  // Yellow background (will be inverted to blue)
#define COLOR_TEXT           0x000000   // Black text (will be inverted to white)
#define COLOR_ACCENT         0x0025FF   // Blue accent (will be inverted to orange)
#define COLOR_RED            0x00FFFF   // Cyan (will be inverted to red)
#define COLOR_GREEN          0xFF00FF   // Magenta (will be inverted to green)
#define COLOR_BLUE           0xFFFF00   // Yellow (will be inverted to blue)

void setup()
{
    Serial.begin(115200);
    Serial.println("=== T-Watch S3 Intermediate Test ===");
    
    // Initialize I2C
    Wire.begin(BOARD_TOUCH_SDA, BOARD_TOUCH_SCL);
    Serial.println("I2C initialized");
    
    // Initialize display
    Serial.println("Initializing display...");
    display.init();
    Serial.println("Display initialized");
    
    display.setRotation(0);
    display.setBrightness(128);
    Serial.println("Display configured");
    
    // Test display with colors
    Serial.println("Testing display colors...");
    
    // Test 1: Red screen
    Serial.println("Test 1: Red screen");
    display.fillScreen(COLOR_RED);
    delay(2000);
    
    // Test 2: Green screen
    Serial.println("Test 2: Green screen");
    display.fillScreen(COLOR_GREEN);
    delay(2000);
    
    // Test 3: Blue screen
    Serial.println("Test 3: Blue screen");
    display.fillScreen(COLOR_BLUE);
    delay(2000);
    
    // Test 4: White screen
    Serial.println("Test 4: White screen");
    display.fillScreen(COLOR_TEXT);
    delay(2000);
    
    // Test 5: Black screen with text
    Serial.println("Test 5: Black screen with text");
    display.fillScreen(COLOR_BACKGROUND);
    display.setTextColor(COLOR_TEXT);
    display.setTextSize(2);
    display.setCursor(20, 80);
    display.println("DISPLAY OK!");
    display.setCursor(20, 110);
    display.println("TOUCH TEST");
    display.setCursor(20, 140);
    display.println("TOUCH SCREEN");
    
    Serial.println("Display test completed");
    Serial.println("Touch the screen to test...");
}

void loop()
{
    // Test touch
    uint16_t x, y;
    if (display.getTouch(&x, &y)) {
        Serial.printf("Touch detected at (%d, %d)\n", x, y);
        
        // Show touch feedback
        display.fillCircle(x, y, 10, COLOR_RED);
        delay(500);
        display.fillCircle(x, y, 10, COLOR_BACKGROUND);
    }
    
    delay(100);
} 