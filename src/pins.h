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
