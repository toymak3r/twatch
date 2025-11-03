#include <LovyanGFX.hpp>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <XPowersLib.h>
#include <SensorBMA423.hpp>
#include <SensorPCF8563.hpp>
#include "esp_sntp.h"
#include <FFat.h>
#include <FS.h>
#include "wifi_icon.h"
#include "wifi_green_icon.h"

// Enable TrueType font support
#define LOAD_GFXFF
#define GFXFF 1

// -------------------- Board pin definitions (T-Watch S3) --------------------
// LCD
#define BOARD_LCD_SCLK    18  // TFT_SCLK
#define BOARD_LCD_MOSI    13  // TFT_MOSI
#define BOARD_LCD_MISO    -1  // TFT_MISO (Not connected)
#define BOARD_LCD_DC      38  // TFT_DC
#define BOARD_LCD_CS      12  // TFT_CS
#define BOARD_LCD_RST     -1  // TFT_RST (Not connected)
#define BOARD_LCD_BL      45  // TFT_BL (LED back-light)

// Touch screen (FT53xx) — I2C bus on Wire1 (port 1)
#define BOARD_TOUCH_SDA   8
#define BOARD_TOUCH_SCL   9
#define BOARD_TOUCH_INT   16

// PMU/RTC/IMU I2C bus (AXP2101/PCF8563/BMA423) — Wire (port 0)
#define BOARD_I2C_SDA     10
#define BOARD_I2C_SCL     11

// PMU interrupt pin
#define BOARD_PMU_INT     21

// -------------------- Hardware components --------------------
XPowersAXP2101 PMU;
SensorBMA423 BMA;
SensorPCF8563 RTC;

// -------------------- LovyanGFX Display Configuration --------------------
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
            cfg.spi_host   = SPI2_HOST;
            cfg.spi_mode   = 0;
            cfg.freq_write = 40000000;
            cfg.pin_sclk   = BOARD_LCD_SCLK;
            cfg.pin_mosi   = BOARD_LCD_MOSI;
            cfg.pin_miso   = BOARD_LCD_MISO;
            cfg.pin_dc     = BOARD_LCD_DC;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }
        { // Panel configuration
            auto cfg = _panel_instance.config();
            cfg.pin_cs          = BOARD_LCD_CS;
            cfg.pin_rst         = BOARD_LCD_RST;
            cfg.pin_busy        = -1;
            cfg.panel_width     = 240;
            cfg.panel_height    = 240;
            cfg.offset_x        = 0;
            cfg.offset_y        = 0;
            cfg.offset_rotation = 0;
            cfg.rgb_order       = false;
            cfg.invert          = true;     // ST7789 needs inversion
            cfg.readable        = false;
            _panel_instance.config(cfg);
        }
        { // Backlight
            auto cfg = _light_instance.config();
            cfg.pin_bl      = BOARD_LCD_BL;
            cfg.invert      = false;
            cfg.freq        = 44100;
            cfg.pwm_channel = 7;
            _light_instance.config(cfg);
            _panel_instance.setLight(&_light_instance);
        }
        { // Touch configuration (FT5x06/FT5336)
            auto cfg = _touch_instance.config();
            cfg.i2c_port = 1;                   // <— use I2C port 1 (Wire1)
            cfg.pin_sda  = BOARD_TOUCH_SDA;     // 8
            cfg.pin_scl  = BOARD_TOUCH_SCL;     // 9
            cfg.pin_int  = BOARD_TOUCH_INT;     // 16
            cfg.i2c_addr = 0x38;
            cfg.freq     = 400000;              // 400 kHz
            _touch_instance.config(cfg);
            _panel_instance.setTouch(&_touch_instance);
        }
        setPanel(&_panel_instance);
    }
};

LGFX display;

// -------------------- Colors --------------------
#define COLOR_RED            0xF800
#define COLOR_GREEN          0x07E0
#define COLOR_BLUE           0x001F
#define COLOR_WHITE          0xFFFF
#define COLOR_BLACK          0x0000
#define COLOR_YELLOW         0xFFE0
#define COLOR_ORANGE         0xFD20
#define COLOR_PURPLE         0xF81F
#define COLOR_CYAN           0x07FF
#define COLOR_DARK_GRAY      0x4208
#define COLOR_LIGHT_GRAY     0x8410

#define COLOR_CUSTOM_BG      0x0000
#define COLOR_CUSTOM_TEXT    0xF800
#define COLOR_CUSTOM_ACCENT  0xFFE0
#define COLOR_CUSTOM_WARNING 0xF800
#define COLOR_CUSTOM_SUCCESS 0x07E0
#define COLOR_CUSTOM_HIGHLIGHT 0xFFFF

#define COLOR_TEXT_AA1       0x8000
#define COLOR_TEXT_AA2       0x4000
#define COLOR_TEXT_AA3       0x2000

#define COLOR_BACKGROUND     COLOR_CUSTOM_BG
#define COLOR_TEXT           COLOR_CUSTOM_TEXT

// -------------------- WiFi network structure --------------------
struct WiFiNetwork {
    char ssid[32];
    char password[64];
    bool valid;
};

// -------------------- Globals --------------------
struct tm timeinfo;
uint32_t stepCounter = 0;
bool wifiConnected = false;
WiFiNetwork wifiNetworks[4];
int wifiNetworkCount = 0;

// Battery variables
bool pmuInitialized = false;
float batteryVoltage = 0.0;
int batteryPercent = 0;
bool isCharging = false;
bool isBatteryConnected = false;
float batteryCurrent = 0.0;
float batteryPower = 0.0;

// Anti-flicker variables
int lastSecond = -1;
int lastBatteryPercent = -1;
bool lastChargingState = false;
uint32_t lastStepCount = 0;
int lastMinute = -1;
int lastHour = -1;
int lastDay = -1;
int lastMonth = -1;
bool lastWifiState = false;
char lastTimeStr[20] = "";
char lastDateStr[20] = "";
char lastDayStr[20] = "";
char lastBatteryStr[16] = "";
char lastWifiStr[16] = "";
bool needsFullRedraw = true;

// Sleep mode variables
bool displaySleep = false;
unsigned long lastActivity = 0;
unsigned long sleepStartTime = 0;
const unsigned long MAX_SLEEP_TIME = 300000;  // 5 min watchdog

// Weather
bool weatherInitialized = false;
float weatherTemperature = 0.0;
String weatherCity = "São Paulo";
String weatherCountry = "BR";
String weatherApiKey = "";
String weatherDescription = "";
String weatherUnits = "metric";
unsigned long lastWeatherUpdate = 0;
unsigned long weatherUpdateInterval = 300000; // 5 min
bool weatherConfigLoaded = false;

// WiFi icon
bool wifiImageLoaded = false;

// Sleep thresholds
const unsigned long SLEEP_TIMEOUT = 15000;
const unsigned long DEEP_SLEEP_TIMEOUT = 45000;
bool deepSleepMode = false;
bool forceRedraw = false;

// WiFi power mgmt
const unsigned long WIFI_CONNECT_TIMEOUT = 10000;
const unsigned long WIFI_IDLE_TIMEOUT = 30000;
const unsigned long WIFI_RECONNECT_INTERVAL = 60000;
unsigned long lastWiFiActivity = 0;
unsigned long lastWiFiReconnectAttempt = 0;
bool wifiPowerSaving = true;
int wifiReconnectAttempts = 0;
const int MAX_WIFI_RECONNECT_ATTEMPTS = 3;

// Touch
bool touchInitialized = false;
uint16_t lastTouchX = 0, lastTouchY = 0;
bool touchDetected = false;

// PMU IRQ
bool pmuIrqFlag = false;

// NTP
#define NTP_SERVER1           "pool.ntp.org"
#define NTP_SERVER2           "time.nist.gov"
#define GMT_OFFSET_SEC        0
#define DAY_LIGHT_OFFSET_SEC  (3600)

// -------------------- Utils --------------------
String getValue(String data, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    int maxIndex = data.length() - 1;
    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// -------------------- Weather config --------------------
bool loadWeatherConfig() {
    Serial.println("Loading weather configuration...");
    if (!FFat.begin(true)) { Serial.println("FATFS init failed for weather config!"); return false; }
    if (!FFat.exists("/weather_config.ini")) { Serial.println("Weather config file not found!"); return false; }
    File file = FFat.open("/weather_config.ini", "r");
    if (!file) { Serial.println("Failed to open weather config file!"); return false; }

    String line;
    while (file.available()) {
        line = file.readStringUntil('\n'); line.trim();
        if (line.startsWith("#") || line.length() == 0) continue;
        if (line.startsWith("api_key=")) { weatherApiKey = getValue(line, '=', 1); }
        if (line.startsWith("city="))    { weatherCity   = getValue(line, '=', 1); }
        if (line.startsWith("country=")) { weatherCountry= getValue(line, '=', 1); }
        if (line.startsWith("update_interval=")) {
            String intervalStr = getValue(line, '=', 1);
            weatherUpdateInterval = intervalStr.toInt() * 1000UL;
        }
        if (line.startsWith("units="))   { weatherUnits  = getValue(line, '=', 1); }
    }
    file.close();

    if (weatherApiKey.length() > 0 && weatherApiKey != "YOUR_API_KEY_HERE") {
        weatherConfigLoaded = true;
        Serial.println("Weather configuration loaded successfully");
        return true;
    } else {
        Serial.println("Weather API key not configured!");
        return false;
    }
}

// -------------------- WiFi config --------------------
bool loadWiFiConfig() {
    Serial.println("Starting WiFi config load...");
    if (!FFat.begin(true)) { Serial.println("FATFS initialization failed!"); return false; }
    if (!FFat.exists("/wifi_config.ini")) {
        Serial.println("WiFi config file not found!");
        File root = FFat.open("/");
        File file = root.openNextFile();
        while (file) {
            Serial.printf("  %s (%d bytes)\n", file.name(), file.size());
            file = root.openNextFile();
        }
        return false;
    }
    File file = FFat.open("/wifi_config.ini", "r");
    if (!file) { Serial.println("Failed to open WiFi config file!"); return false; }

    wifiNetworkCount = 0;
    String line;
    while (file.available() && wifiNetworkCount < 4) {
        line = file.readStringUntil('\n'); line.trim();
        if (line.startsWith("#") || line.length() == 0) continue;

        if (line.startsWith("wifi") && line.indexOf("_ssid=") > 0) {
            String ssid = getValue(line, '=', 1);
            if (ssid.length() > 0) {
                strncpy(wifiNetworks[wifiNetworkCount].ssid, ssid.c_str(), 31);
                wifiNetworks[wifiNetworkCount].ssid[31] = '\0';
                wifiNetworks[wifiNetworkCount].valid = false;
            }
        }
        if (line.startsWith("wifi") && line.indexOf("_password=") > 0) {
            String password = getValue(line, '=', 1);
            if (password.length() > 0 && wifiNetworkCount < 4) {
                strncpy(wifiNetworks[wifiNetworkCount].password, password.c_str(), 63);
                wifiNetworks[wifiNetworkCount].password[63] = '\0';
                wifiNetworks[wifiNetworkCount].valid = true;
                wifiNetworkCount++;
            }
        }
    }
    file.close();
    Serial.printf("Loaded %d WiFi networks from config file\n", wifiNetworkCount);
    for (int i = 0; i < wifiNetworkCount; i++) {
        Serial.printf("Network %d: %s\n", i + 1, wifiNetworks[i].ssid);
    }
    return wifiNetworkCount > 0;
}

// -------------------- Battery/PMU --------------------
void readBatteryInfo() {
    if (!pmuInitialized) {
        Serial.println("PMU not initialized, using fallback battery reading");
        batteryVoltage = analogRead(1) * 2 * 3.3 / 4095.0;
        if (batteryVoltage >= 4.2) batteryPercent = 100;
        else if (batteryVoltage >= 3.8) batteryPercent = (int)((batteryVoltage - 3.8) * 250);
        else if (batteryVoltage >= 3.4) batteryPercent = (int)((batteryVoltage - 3.4) * 125);
        else if (batteryVoltage >= 3.0) batteryPercent = (int)((batteryVoltage - 3.0) * 125);
        else batteryPercent = 0;
        batteryPercent = constrain(batteryPercent, 0, 100);
        Serial.printf("Fallback calculation: %.2fV = %d%%\n", batteryVoltage, batteryPercent);
        return;
    }

    // getBattVoltage() returns millivolts, not volts - convert to volts
    batteryVoltage     = PMU.getBattVoltage() / 1000.0;  // Convert mV to V
    int pmuPercent     = PMU.getBatteryPercent();
    isBatteryConnected = PMU.isBatteryConnect();
    isCharging         = PMU.isCharging();
    bool isVbusIn      = PMU.isVbusIn();

    if (batteryVoltage >= 4.2) batteryPercent = 100;
    else if (batteryVoltage >= 3.8) batteryPercent = (int)((batteryVoltage - 3.8) * 250);
    else if (batteryVoltage >= 3.4) batteryPercent = (int)((batteryVoltage - 3.4) * 125);
    else if (batteryVoltage >= 3.0) batteryPercent = (int)((batteryVoltage - 3.0) * 125);
    else batteryPercent = 0;
    batteryPercent = constrain(batteryPercent, 0, 100);

    if (batteryPercent == 0 && pmuPercent > 0) batteryPercent = pmuPercent;

    Serial.printf("Battery: %.2fV, %d%%, Charging: %s, Connected: %s, VBUS: %s\n",
                 batteryVoltage, batteryPercent,
                 isCharging ? "Yes" : "No",
                 isBatteryConnected ? "Yes" : "No",
                 isVbusIn ? "Yes" : "No");

    if ((isCharging || isVbusIn) && batteryVoltage > 3.5 && batteryPercent == 0) {
        batteryPercent = 1;
    }
}

void optimizeBatteryUsage() {
    if (!pmuInitialized) return;

    if (batteryPercent <= 20) {
        display.setBrightness(64);
    } else if (batteryPercent <= 50) {
        display.setBrightness(128);
    } else {
        display.setBrightness(200);
    }

    if (batteryPercent <= 10 && wifiConnected) {
        WiFi.disconnect(); wifiConnected = false;
    }

    if (batteryPercent <= 15)        setCpuFrequencyMhz(80);
    else if (batteryPercent <= 30)   setCpuFrequencyMhz(160);
    else                              setCpuFrequencyMhz(240);
}

// -------------------- WiFi --------------------
void disconnectWiFi() {
    if (wifiConnected) {
        Serial.println("Disconnecting WiFi to save power...");
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        wifiConnected = false;
        Serial.println("WiFi disconnected successfully");
    }
}

bool connectToWiFi();

void connectWiFi() {
    if (wifiConnected) { Serial.println("WiFi already connected"); return; }
    if (wifiNetworkCount == 0) {
        Serial.println("No WiFi networks configured! Loading config first...");
        if (!loadWiFiConfig()) { Serial.println("Failed to load WiFi configuration!"); return; }
    }
    Serial.println("Connecting to WiFi...");

    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_STA);

    if (wifiPowerSaving) { WiFi.setSleep(true); }

    if (wifiNetworkCount > 0) {
        WiFi.begin(wifiNetworks[0].ssid, wifiNetworks[0].password);
        Serial.printf("Connecting to: %s\n", wifiNetworks[0].ssid);
    } else {
        Serial.println("No WiFi networks configured!");
        return;
    }

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_CONNECT_TIMEOUT) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        lastWiFiActivity = millis();
        wifiReconnectAttempts = 0;
        Serial.printf("\nWiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());
        configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);
        Serial.println("Time sync configured");
    } else {
        Serial.println("\nWiFi connection failed!");
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
    }
}

bool connectToWiFi() { connectWiFi(); return wifiConnected; }

// -------------------- PNG icon --------------------
bool loadWiFiImage() {
    if (wifiImageLoaded) return true;
    Serial.println("Loading WiFi image from embedded data...");
    bool success = display.drawPng(data_assets_wifi_small_png, data_assets_wifi_small_png_len, -100, -100);
    if (success) {
        wifiImageLoaded = true;
        Serial.printf("WiFi PNG image loaded successfully! Size: %d bytes\n", data_assets_wifi_small_png_len);
        return true;
    } else {
        Serial.println("PNG loading failed, will use vector icon");
        return false;
    }
}

// -------------------- Weather --------------------
void getWeatherData() {
    if (!wifiConnected) {
        Serial.println("WiFi not connected, attempting to connect for weather data...");
        connectWiFi();
        if (!wifiConnected) { Serial.println("Failed to connect WiFi for weather data"); return; }
    }
    if (!weatherConfigLoaded) {
        Serial.println("Weather configuration not loaded, cannot get weather data"); return;
    }

    unsigned long currentTime = millis();
    unsigned long timeSinceUpdate = (currentTime >= lastWeatherUpdate)
                                  ? (currentTime - lastWeatherUpdate)
                                  : (0xFFFFFFFF - lastWeatherUpdate + currentTime);

    if (lastWeatherUpdate != 0 && timeSinceUpdate < weatherUpdateInterval) {
        Serial.printf("Weather update not due yet. Next update in %lu ms\n",
                      weatherUpdateInterval - timeSinceUpdate);
        return;
    }

    if (weatherApiKey.length() == 0 || weatherCity.length() == 0) {
        Serial.println("ERROR: Weather config incomplete!");
        return;
    }

    HTTPClient http;
    String location = weatherCity; location.replace(" ", "%20");
    if (weatherCountry.length() > 0) location += "," + weatherCountry;
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + location +
                 "&appid=" + weatherApiKey + "&units=" + weatherUnits;

    Serial.printf("Weather API URL: %s\n", url.c_str());
    http.begin(url); http.setTimeout(10000);
    int httpCode = http.GET();
    Serial.printf("HTTP response code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        lastWiFiActivity = millis();

        int tempIndex = payload.indexOf("\"temp\":");
        if (tempIndex > 0) {
            int tempStart = payload.indexOf(":", tempIndex) + 1;
            int tempEnd = payload.indexOf(",", tempStart);
            if (tempEnd == -1) tempEnd = payload.indexOf("}", tempStart);
            String tempStr = payload.substring(tempStart, tempEnd); tempStr.trim();
            weatherTemperature = tempStr.toFloat();
            if (weatherTemperature != 0 || tempStr == "0") {
                weatherInitialized = true;
            }
        }
        int descIndex = payload.indexOf("\"description\":\"");
        if (descIndex > 0) {
            int descStart = descIndex + 15;
            int descEnd = payload.indexOf("\"", descStart);
            weatherDescription = payload.substring(descStart, descEnd);
        }
    } else {
        String errorPayload = http.getString();
        Serial.printf("HTTP request failed, error: %d\n", httpCode);
        Serial.printf("Error response: %s\n", errorPayload.c_str());
    }

    http.end();
    lastWeatherUpdate = millis();
    Serial.printf("Weather update completed. Next update in %lu seconds (%lu ms)\n", 
                  weatherUpdateInterval / 1000, weatherUpdateInterval);
}

// -------------------- Text helpers (Lovyan fonts) --------------------
void drawTrueTypeText(const char* text, int x, int y, int fontSize, uint16_t color) {
    display.setTextColor(color);
    switch(fontSize) {
        case 1: display.setFont(&fonts::Font0); display.setTextSize(1); break;
        case 2: display.setFont(&fonts::Font2); display.setTextSize(1); break;
        case 3: display.setFont(&fonts::Font4); display.setTextSize(1); break;
        case 4: display.setFont(&fonts::Font6); display.setTextSize(1); break;
        case 5: display.setFont(&fonts::Font6); display.setTextSize(1.3); break;
        default: display.setFont(&fonts::Font2); display.setTextSize(1); break;
    }
    display.setCursor(x, y);
    display.print(text);
}

int getTrueTypeTextWidth(const char* text, int fontSize) {
    switch(fontSize) {
        case 1: display.setFont(&fonts::Font0); display.setTextSize(1); break;
        case 2: display.setFont(&fonts::Font2); display.setTextSize(1); break;
        case 3: display.setFont(&fonts::Font4); display.setTextSize(1); break;
        case 4: display.setFont(&fonts::Font6); display.setTextSize(1); break;
        case 5: display.setFont(&fonts::Font6); display.setTextSize(1.3); break;
        default: display.setFont(&fonts::Font2); display.setTextSize(1); break;
    }
    return display.textWidth(text);
}

int getTrueTypeFontHeight(int fontSize) {
    switch(fontSize) {
        case 1: display.setFont(&fonts::Font0); display.setTextSize(1); break;
        case 2: display.setFont(&fonts::Font2); display.setTextSize(1); break;
        case 3: display.setFont(&fonts::Font4); display.setTextSize(1); break;
        case 4: display.setFont(&fonts::Font6); display.setTextSize(1); break;
        case 5: display.setFont(&fonts::Font6); display.setTextSize(1.3); break;
        default: display.setFont(&fonts::Font2); display.setTextSize(1); break;
    }
    return display.fontHeight();
}

// -------------------- UI bits --------------------
void drawStepsIcon(int x, int y) {
    display.fillRect(x, y + 6, 3, 3, COLOR_CUSTOM_ACCENT);
    display.fillRect(x + 3, y + 3, 3, 3, COLOR_CUSTOM_ACCENT);
    display.fillRect(x + 6, y, 3, 3, COLOR_CUSTOM_ACCENT);
    display.fillRect(x + 9, y + 3, 3, 3, COLOR_CUSTOM_ACCENT);
    display.fillRect(x + 12, y + 6, 3, 3, COLOR_CUSTOM_ACCENT);
}

void drawWiFiIcon(int x, int y, bool isConnected);
void drawVectorWiFiIcon(int x, int y, bool isConnected) {
    uint16_t iconColor = isConnected ? COLOR_GREEN : COLOR_RED;
    int centerX = x, centerY = y;

    for (int i = 0; i < 3; i++) {
        int radius = 8 + i * 4;
        int startAngle = 225;
        int endAngle = 315;
        for (int angle = startAngle; angle <= endAngle; angle += 5) {
            int x1 = centerX + (radius - 1) * cos(angle * PI / 180);
            int y1 = centerY + (radius - 1) * sin(angle * PI / 180);
            int x2 = centerX + (radius + 1) * cos(angle * PI / 180);
            int y2 = centerY + (radius + 1) * sin(angle * PI / 180);
            display.drawLine(x1, y1, x2, y2, iconColor);
        }
    }
    display.fillCircle(centerX, centerY, 2, iconColor);
    if (isConnected) {
        display.fillCircle(centerX, centerY - 12, 2, COLOR_GREEN);
    } else {
        display.drawLine(centerX - 3, centerY - 9, centerX + 3, centerY - 15, COLOR_RED);
        display.drawLine(centerX + 3, centerY - 9, centerX - 3, centerY - 15, COLOR_RED);
    }
}

void drawWiFiIcon(int x, int y, bool isConnected) {
    if (wifiImageLoaded) {
        int pngX = x - 9, pngY = y - 7;
        bool success = false;
        if (isConnected) {
            success = display.drawPng(data_assets_wifi_green_png, data_assets_wifi_green_png_len, pngX, pngY);
        } else {
            success = display.drawPng(data_assets_wifi_small_png, data_assets_wifi_small_png_len, pngX, pngY);
        }
        if (success) return;
    }
    drawVectorWiFiIcon(x, y, isConnected);
}

// -------------------- Sleep/wake --------------------
void enterSleepMode() {
    if (!displaySleep) {
        Serial.println("Entering sleep mode...");
        displaySleep = true;
        sleepStartTime = millis();
        display.setBrightness(0);
        display.fillScreen(COLOR_BACKGROUND);
        if (wifiConnected) { WiFi.disconnect(); wifiConnected = false; }
        setCpuFrequencyMhz(80);
    }
}

void exitSleepMode() {
    if (displaySleep) {
        Serial.println("Exiting sleep mode...");
        displaySleep = false; deepSleepMode = false;
        lastActivity = millis(); sleepStartTime = 0;
        setCpuFrequencyMhz(240);
        delay(50);

        if (!pmuInitialized) {
            Serial.println("Reinitializing PMU...");
            Wire.end(); delay(100);
            Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL, 400000);
            delay(100);
            bool pmuFound = false;
            uint8_t pmuAddresses[] = {AXP2101_SLAVE_ADDRESS, 0x34, 0x35, 0x36};
            for (int i = 0; i < 4; i++) {
                if (PMU.begin(Wire, pmuAddresses[i], BOARD_I2C_SDA, BOARD_I2C_SCL)) {
                    pmuInitialized = true; pmuFound = true; break;
                }
                delay(50);
            }
            if (!pmuFound) {
                Serial.println("PMU reinit failed");
                batteryVoltage = analogRead(1) * 2 * 3.3 / 4095.0;
            }
        }

        readBatteryInfo();
        uint8_t targetBrightness = (batteryPercent <= 20) ? 64 : (batteryPercent <= 50) ? 128 : 200;
        for (int i = 0; i < 3; i++) { display.setBrightness(targetBrightness); delay(20); }
        display.fillScreen(COLOR_BACKGROUND); delay(50);

        if (!wifiConnected && wifiNetworkCount > 0) {
            Serial.println("Reconnecting WiFi...");
            WiFi.disconnect(); WiFi.mode(WIFI_OFF); delay(200); WiFi.mode(WIFI_STA);
            if (connectToWiFi()) {
                configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);
                lastWeatherUpdate = 0;
            }
        }
        needsFullRedraw = true;
        Serial.println("Sleep mode exit completed");
    }
}

void checkActivity() {
    // Only update activity tracking when awake - power button is the only wake-up source
    if (displaySleep) {
        // Skip all sensor checks when sleeping - only power button can wake
        static unsigned long lastTouchDebug = 0;
        if (millis() - lastTouchDebug > 5000) {
            lastTouchDebug = millis();
            Serial.println("Sleep mode: waiting for power button press…");
        }
        return;
    }
    
    // Activity detection when display is awake
    if (BMA.isPedometer()) {
        uint32_t currentSteps = BMA.getPedometerCounter();
        if (currentSteps != lastStepCount) {
            lastActivity = millis(); 
            lastStepCount = currentSteps;
        }
    }

    static int16_t lastAccelX = 0, lastAccelY = 0, lastAccelZ = 0;
    static unsigned long lastAccelRead = 0;
    int16_t rawBuffer[3];
    if (millis() - lastAccelRead > 1000) {
        lastAccelRead = millis();
        if (BMA.getAccelRaw(rawBuffer)) {
            int16_t ax = rawBuffer[0], ay = rawBuffer[1], az = rawBuffer[2];
            int32_t movement = abs(ax - lastAccelX) + abs(ay - lastAccelY) + abs(az - lastAccelZ);
            if (movement > 100) {
                lastActivity = millis();
            }
            lastAccelX = ax; lastAccelY = ay; lastAccelZ = az;
        }
    }

    static unsigned long lastTouchCheck = 0;
    if (millis() - lastTouchCheck > 500) {
        lastTouchCheck = millis();
        // Use Wire1 for touch + repeated-start reads
        Wire1.beginTransmission(0x38);
        Wire1.write(0x02);
        if (Wire1.endTransmission(false) == 0) {
            if (Wire1.requestFrom((uint8_t)0x38, (size_t)1, true) == 1) {
                uint8_t touchPoints = Wire1.read() & 0x0F;
                if (touchPoints > 0) {
                    Wire1.beginTransmission(0x38);
                    Wire1.write(0x03);
                    if (Wire1.endTransmission(false) == 0) {
                        if (Wire1.requestFrom((uint8_t)0x38, (size_t)6, true) == 6) {
                            uint8_t d[6];
                            for (int i = 0; i < 6; ++i) d[i] = Wire1.read();
                            uint16_t x = ((d[0] & 0x0F) << 8) | d[1];
                            uint16_t y = ((d[2] & 0x0F) << 8) | d[3];
                            if (abs((int)x - (int)lastTouchX) > 5 || abs((int)y - (int)lastTouchY) > 5) {
                                lastActivity = millis();
                                lastTouchX = x; lastTouchY = y; touchDetected = true;
                            }
                        }
                    }
                } else { touchDetected = false; }
            }
        }
    }
}

void manageSleepMode() {
    unsigned long currentTime = millis();
    static unsigned long lastSleepDebug = 0;

    if (displaySleep && sleepStartTime > 0) {
        unsigned long sleepDuration = currentTime - sleepStartTime;
        if (sleepDuration > MAX_SLEEP_TIME) {
            Serial.printf("WATCHDOG: slept %lu ms (max %lu) — forcing wake\n", sleepDuration, MAX_SLEEP_TIME);
            exitSleepMode(); return;
        }
    }

    if (millis() - lastSleepDebug > 5000) {
        lastSleepDebug = millis();
        unsigned long since = currentTime - lastActivity;
        Serial.printf("Sleep timer: %lu ms since last activity, timeout: %lu ms, sleep: %s\n",
                      since, SLEEP_TIMEOUT, displaySleep ? "YES" : "NO");
        if (displaySleep && sleepStartTime > 0) {
            Serial.printf("  Sleep duration: %lu ms (max: %lu ms)\n", currentTime - sleepStartTime, MAX_SLEEP_TIME);
        }
    }

    if (!displaySleep && (currentTime - lastActivity > SLEEP_TIMEOUT)) {
        Serial.printf("Sleep timeout reached! Entering sleep after %lu ms\n", currentTime - lastActivity);
        enterSleepMode();
    }
    if (displaySleep && !deepSleepMode && (currentTime - lastActivity > DEEP_SLEEP_TIMEOUT)) {
        Serial.println("Entering deep sleep mode...");
        deepSleepMode = true;
        if (wifiConnected) { WiFi.disconnect(); wifiConnected = false; }
        setCpuFrequencyMhz(80);
    }
    checkActivity();
}

// -------------------- WiFi Power Mgmt --------------------
void manageWiFiPower() {
    // Skip WiFi operations when display is sleeping to prevent wake-ups
    if (displaySleep) {
        return;
    }
    
    unsigned long now = millis();
    if (wifiConnected && wifiPowerSaving) {
        if (now - lastWiFiActivity > WIFI_IDLE_TIMEOUT) {
            Serial.println("WiFi idle timeout - disconnecting to save power");
            disconnectWiFi();
            return;
        }
    }
    if (!wifiConnected && weatherConfigLoaded) {
        if (now - lastWiFiReconnectAttempt > WIFI_RECONNECT_INTERVAL) {
            if (wifiReconnectAttempts < MAX_WIFI_RECONNECT_ATTEMPTS) {
                Serial.printf("Attempting WiFi reconnection (%d/%d)\n",
                              wifiReconnectAttempts + 1, MAX_WIFI_RECONNECT_ATTEMPTS);
                connectWiFi();
                lastWiFiReconnectAttempt = now;
                wifiReconnectAttempts++;
            } else {
                Serial.println("Max WiFi reconnection attempts reached - will retry later");
                if (now - lastWiFiReconnectAttempt > 300000) { // 5 min
                    wifiReconnectAttempts = 0;
                }
            }
        }
    }
}

// -------------------- UI --------------------
void drawWiFiIcon(int x, int y, bool isConnected);

void drawCustomInterface() {
    if (displaySleep) return;
    if (!getLocalTime(&timeinfo)) { Serial.println("Failed to obtain time"); return; }

    char timeStr[20]; sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    char dateStr[20]; sprintf(dateStr, "%02d/%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1);
    char dayStr[20];  const char* days[] = {"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"}; strcpy(dayStr, days[timeinfo.tm_wday]);
    char batteryStr[16]; sprintf(batteryStr, "%d%%", batteryPercent);
    char wifiStr[16]; strcpy(wifiStr, wifiConnected ? "WiFi ON" : "WiFi OFF");

    bool needsRedraw = needsFullRedraw ||
                       strcmp(timeStr, lastTimeStr) != 0 ||
                       strcmp(dateStr, lastDateStr) != 0 ||
                       strcmp(dayStr, lastDayStr) != 0 ||
                       batteryPercent != lastBatteryPercent ||
                       isCharging != lastChargingState ||
                       stepCounter != lastStepCount ||
                       wifiConnected != lastWifiState;

    if (!needsRedraw) return;

    if (needsFullRedraw) { display.fillScreen(COLOR_BACKGROUND); needsFullRedraw = false; }

    int screenWidth = display.width(), screenHeight = display.height();
    int timeWidth = getTrueTypeTextWidth(timeStr, 5);
    int timeX = (screenWidth - timeWidth) / 2;
    int timeY = 70;

    int dateWidth = getTrueTypeTextWidth(dateStr, 2);
    int dateX = (screenWidth - dateWidth) / 2;
    int dateY = timeY + getTrueTypeFontHeight(4) + 10;

    int dayWidth = getTrueTypeTextWidth(dayStr, 2);
    int dayX = (screenWidth - dayWidth) / 2;
    int dayY = dateY + getTrueTypeFontHeight(2) + 5;

    int batteryWidth = getTrueTypeTextWidth(batteryStr, 1);
    int batteryX = screenWidth - batteryWidth - 10;
    int batteryY = 10;

    int wifiX = 25, wifiY = 25;

    char stepsStr[16]; sprintf(stepsStr, "Steps: %lu", stepCounter);
    int stepsWidth = getTrueTypeTextWidth(stepsStr, 1);
    int stepsX = (screenWidth - stepsWidth) / 2;
    int stepsY = screenHeight - getTrueTypeFontHeight(1) - 10;

    char tempStr[16];
    if (weatherConfigLoaded && weatherInitialized) sprintf(tempStr, "%.1f°C", weatherTemperature);
    else if (weatherConfigLoaded) strcpy(tempStr, "N/A");
    else strcpy(tempStr, "NO API");
    int tempWidth = getTrueTypeTextWidth(tempStr, 1);
    int tempX = screenWidth - tempWidth - 10;
    int tempY = screenHeight - getTrueTypeFontHeight(1) - 10;

    if (strcmp(timeStr, lastTimeStr) != 0) {
        display.fillRect(timeX - 5, timeY - 5, timeWidth + 10, getTrueTypeFontHeight(4) + 10, COLOR_BACKGROUND);
    }
    if (strcmp(dateStr, lastDateStr) != 0) {
        display.fillRect(dateX - 5, dateY - 5, dateWidth + 10, getTrueTypeFontHeight(2) + 10, COLOR_BACKGROUND);
    }
    if (strcmp(dayStr, lastDayStr) != 0) {
        display.fillRect(dayX - 5, dayY - 5, dayWidth + 10, getTrueTypeFontHeight(2) + 10, COLOR_BACKGROUND);
    }
    if (batteryPercent != lastBatteryPercent || isCharging != lastChargingState) {
        display.fillRect(batteryX - 5, batteryY - 5, batteryWidth + 10, getTrueTypeFontHeight(1) + 10, COLOR_BACKGROUND);
    }
    if (wifiConnected != lastWifiState) {
        display.fillRect(wifiX - 15, wifiY - 15, 30, 30, COLOR_BACKGROUND);
    }
    if (stepCounter != lastStepCount) {
        display.fillRect(stepsX - 5, stepsY - 5, stepsWidth + 10, getTrueTypeFontHeight(1) + 10, COLOR_BACKGROUND);
    }

    drawTrueTypeText(timeStr, timeX, timeY, 4, COLOR_TEXT);
    drawTrueTypeText(dateStr, dateX, dateY, 2, COLOR_TEXT);
    drawTrueTypeText(dayStr, dayX, dayY, 2, COLOR_TEXT);

    uint16_t batteryColor = (batteryPercent <= 20) ? COLOR_RED : (batteryPercent <= 50) ? COLOR_ORANGE : COLOR_GREEN;
    drawTrueTypeText(batteryStr, batteryX, batteryY, 1, batteryColor);

    drawWiFiIcon(wifiX, wifiY, wifiConnected);
    drawTrueTypeText(stepsStr, stepsX, stepsY, 1, COLOR_TEXT);
    drawTrueTypeText(tempStr, tempX, tempY, 1, COLOR_TEXT);

    strcpy(lastTimeStr, timeStr);
    strcpy(lastDateStr, dateStr);
    strcpy(lastDayStr, dayStr);
    lastBatteryPercent = batteryPercent;
    lastChargingState  = isCharging;
    lastStepCount      = stepCounter;
    lastWifiState      = wifiConnected;

    Serial.printf("📱 Display: Time: %s, Date: %s, Day: %s, Battery: %d%%, WiFi: %s, Steps: %lu, Weather: %.1f°C\n",
                  timeStr, dateStr, dayStr, batteryPercent, wifiStr, stepCounter, weatherTemperature);
}

// -------------------- Setup --------------------
void setup()
{
    Serial.begin(115200);
    Serial.println("T-Watch S3 Starting with TrueType fonts...");

    // Init I2C buses FIRST
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL, 400000);     // PMU/RTC/IMU
    Wire1.begin(BOARD_TOUCH_SDA, BOARD_TOUCH_SCL, 400000); // Touch
    pinMode(BOARD_PMU_INT, INPUT_PULLUP);
    pinMode(BOARD_TOUCH_INT, INPUT);
    
    // Perform I2C bus scan to diagnose hardware issues
    Serial.println("Scanning I2C bus for devices...");
    int deviceCount = 0;
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("-> I2C device found at address 0x%02X\n", address);
            deviceCount++;
            // Identify known devices
            if (address == 0x34) Serial.println("    ^ This is likely the PMU (AXP2101)");
            else if (address == 0x18) Serial.println("    ^ This is likely the BMA423 (default address)");
            else if (address == 0x19) Serial.println("    ^ This might be BMA423 (alternative address)");
            else if (address == 0x51) Serial.println("    ^ This is likely the RTC (PCF8563)");
        }
    }
    if (deviceCount == 0) {
        Serial.println("ERROR: No I2C devices found! Check hardware connections.");
        Serial.println("Expected devices:");
        Serial.println("  - PMU (AXP2101): 0x34");
        Serial.println("  - BMA423: 0x18");
        Serial.println("  - RTC (PCF8563): 0x51");
        Serial.println("Possible causes:");
        Serial.println("  - I2C bus not powered (PMU not initialized)");
        Serial.println("  - Wrong pin configuration");
        Serial.println("  - Hardware failure");
    } else {
        Serial.printf("Found %d I2C device(s) - hardware appears connected\n", deviceCount);
    }

    // Display
    display.init();
    display.setRotation(2);
    display.setBrightness(255);
    display.setColorDepth(16);
    display.setFont(&fonts::Font2);
    display.fillScreen(COLOR_BLACK);

    // Test font
    Serial.println("Testing TrueType fonts...");
    auto drawTT = [&](const char* t,int x,int y,int s,uint16_t c){ display.setTextColor(c); drawTrueTypeText(t,x,y,s,c);};
    drawTT("TRUETYPE TEST", 10, 50, 3, COLOR_TEXT);
    drawTT("Fontes TrueType", 10, 80, 2, COLOR_TEXT);
    drawTT("Qualidade Superior!", 10, 110, 1, COLOR_TEXT);
    delay(1500);
    display.fillScreen(COLOR_BLACK);

    // Touch self-test on Wire1 with repeated-start
    Serial.println("Testing touch screen communication...");
    Wire1.beginTransmission(0x38);
    Wire1.write(0x00);
    Wire1.endTransmission(false);
    if (Wire1.requestFrom((uint8_t)0x38, (size_t)1, true) == 1) {
        uint8_t deviceMode = Wire1.read();
        Serial.printf("Touch device mode: 0x%02X\n", deviceMode);
    } else {
        Serial.println("ERROR: No response from touch device on I2C!");
    }
    uint16_t tx, ty;
    if (display.getTouch(&tx, &ty)) {
        lastTouchX = tx; lastTouchY = ty; touchInitialized = true;
        Serial.println("Touch screen initialized successfully");
    } else {
        Serial.println("Touch screen initialization failed, will retry");
    }

    // PMU init
    Serial.println("Initializing PMU...");
    if (PMU.init(Wire, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
        Serial.println("PMU init succeeded");
        pmuInitialized = true;

        PMU.setVbusVoltageLimit(XPOWERS_AXP2101_VBUS_VOL_LIM_4V36);
        PMU.setVbusCurrentLimit(XPOWERS_AXP2101_VBUS_CUR_LIM_900MA);
        PMU.setSysPowerDownVoltage(2600);

        PMU.setALDO1Voltage(3300);
        PMU.setALDO2Voltage(3300);
        PMU.setALDO3Voltage(3300);
        PMU.setALDO4Voltage(3300);
        PMU.setBLDO2Voltage(3300);
        PMU.setDC3Voltage(3300);

        PMU.enableALDO1();
        PMU.enableALDO2();
        PMU.enableALDO3();
        PMU.enableALDO4();
        PMU.enableBLDO2();
        PMU.enableDC3();

        PMU.disableDC2();
        PMU.disableDC4();
        PMU.disableDC5();
        PMU.disableBLDO1();
        PMU.disableCPUSLDO();
        PMU.disableDLDO1();
        PMU.disableDLDO2();

        PMU.setPowerKeyPressOffTime(XPOWERS_POWEROFF_4S);
        PMU.setPowerKeyPressOnTime(XPOWERS_POWERON_128MS);
        PMU.disableTSPinMeasure();

        PMU.enableBattDetection();
        PMU.enableVbusVoltageMeasure();
        PMU.enableBattVoltageMeasure();
        PMU.enableSystemVoltageMeasure();

        PMU.clearIrqStatus();
        PMU.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
        PMU.enableIRQ(XPOWERS_AXP2101_PKEY_SHORT_IRQ | XPOWERS_AXP2101_PKEY_LONG_IRQ);

        pinMode(BOARD_PMU_INT, INPUT_PULLUP);
        attachInterrupt(BOARD_PMU_INT, [](){ pmuIrqFlag = true; Serial.println("PMU interrupt triggered!"); }, FALLING);

        PMU.disableLongPressShutdown();

        readBatteryInfo();
        display.setCursor(20, 140); display.println("PMU: OK");
    } else {
        Serial.println("PMU initialization failed!");
        if (PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
            Serial.println("PMU begin() succeeded");
            pmuInitialized = true;
            readBatteryInfo();
            display.setCursor(20, 140); display.println("PMU: OK (fallback)");
        } else {
            Serial.println("PMU initialization failed on all methods!");
            display.setCursor(20, 140); display.println("PMU: FAIL");
        }
    }

    // BMA423 - I2C scan showed device at 0x19, try both 0x18 (default) and 0x19
    Serial.println("Initializing BMA423...");
    bool bmaInitSuccess = false;
    if (BMA.init(Wire)) {
        Serial.println("BMA423 init succeeded at default address");
        bmaInitSuccess = true;
    } else {
        Serial.println("BMA423 init failed at default address, trying 0x19...");
        // I2C scan found device at 0x19, try that address
        if (BMA.begin(Wire, 0x19, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
            Serial.println("BMA423 begin() succeeded at 0x19");
            bmaInitSuccess = true;
        } else {
            Serial.println("BMA423 init failed at 0x19, trying default BMA423_SLAVE_ADDRESS...");
            // Try the defined BMA423_SLAVE_ADDRESS as last resort
            if (BMA.begin(Wire, BMA423_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
                Serial.println("BMA423 begin() succeeded at BMA423_SLAVE_ADDRESS");
                bmaInitSuccess = true;
            }
        }
    }
    
    if (bmaInitSuccess) {
        BMA.configAccelerometer();
        BMA.enableAccelerometer();
        BMA.enablePedometer();
        display.setCursor(20, 170); display.println("BMA: OK");
        Serial.println("BMA423 configured successfully");
    } else {
        Serial.println("BMA423 initialization failed on all attempts!");
        display.setCursor(20, 170); display.println("BMA: FAIL");
    }

    // RTC
    if (RTC.begin(Wire, PCF8563_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
        Serial.println("RTC initialized");
        display.setCursor(20, 200); display.println("RTC: OK");
    } else {
        Serial.println("RTC initialization failed!");
        display.setCursor(20, 200); display.println("RTC: FAIL");
    }

    delay(1000);

    // WiFi
    display.fillScreen(COLOR_BLACK);
    display.setCursor(20, 80); display.println("Loading WiFi...");
    if (loadWiFiConfig()) {
        display.setCursor(20, 110); display.println("WiFi Config: OK");
        delay(500);
        display.fillScreen(COLOR_BLACK);
        display.setCursor(20, 80); display.println("Initializing WiFi...");
        connectWiFi();
        if (wifiConnected) {
            display.setCursor(20, 110); display.println("WiFi: CONNECTED");
            int tries = 0; while (!getLocalTime(&timeinfo) && tries++ < 10) { delay(500); }
            if (getLocalTime(&timeinfo)) {
                Serial.println("Time synchronized!");
                display.setCursor(20, 140); display.println("Time: SYNCED");
            } else {
                Serial.println("Time sync failed!");
                display.setCursor(20, 140); display.println("Time: FAIL");
            }
        } else {
            display.setCursor(20, 110); display.println("WiFi: FAILED");
        }
    } else {
        display.setCursor(20, 110); display.println("WiFi Config: FAIL");
    }

    delay(800);

    // Weather cfg
    display.fillScreen(COLOR_BLACK);
    display.setCursor(20, 80); display.println("Loading Weather Config...");
    if (loadWeatherConfig()) {
        display.setCursor(20, 110); display.println("Weather Config: OK");
        lastWeatherUpdate = 0;
    } else {
        display.setCursor(20, 110); display.println("Weather Config: FAIL");
    }

    delay(500);
    loadWiFiImage();

    lastActivity = millis();
    Serial.println("Setup completed!");
}

// -------------------- Main loop --------------------
void manageWiFiPower();
void drawCustomInterface();

void loop()
{
    // Check power button - priority check, especially important during sleep
    if (pmuInitialized) {
        // Check interrupt flag first
        if (pmuIrqFlag) {
            pmuIrqFlag = false;
            Serial.println("PMU interrupt detected!");
        }
        
        // Always poll PMU status for power button (more reliable than interrupt during sleep)
        uint32_t pmuIrqStatus = PMU.getIrqStatus();
        if (pmuIrqStatus != 0) {
            Serial.printf("PMU IRQ Status: 0x%08X\n", pmuIrqStatus);
            bool handled = false;
            if (PMU.isPekeyShortPressIrq()) {
                Serial.println("Power button short press detected - WAKING");
                lastActivity = millis();
                if (displaySleep) {
                    exitSleepMode();
                }
                handled = true;
            }
            if (PMU.isPekeyLongPressIrq()) {
                Serial.println("Power button long press detected");
                lastActivity = millis();
                handled = true;
            }
            // Always clear interrupt status after handling
            PMU.clearIrqStatus();
        }
    }

    manageSleepMode();
    manageWiFiPower();

    if (!displaySleep) {
        static unsigned long lastBatteryRead = 0;
        static bool weatherFirstUpdate = true;

        if (millis() - lastBatteryRead > 8000) {
            readBatteryInfo();
            if (weatherFirstUpdate) {
                lastWeatherUpdate = 0;  // Force first update by resetting timestamp
                weatherFirstUpdate = false;
            }
            getWeatherData();
            optimizeBatteryUsage();
            lastBatteryRead = millis();
        }

        bool needsUpdate = needsFullRedraw ||
                          (timeinfo.tm_min != lastMinute) ||
                          (timeinfo.tm_hour != lastHour) ||
                          (timeinfo.tm_mday != lastDay) ||
                          (timeinfo.tm_mon != lastMonth) ||
                          (wifiConnected != lastWifiState) ||
                          (batteryPercent != lastBatteryPercent) ||
                          touchDetected;

        if (needsUpdate) {
            drawCustomInterface();
        }
        delay(1500);
    } else {
        // During sleep, check power button more frequently
        delay(100);
    }
}
