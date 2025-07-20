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

// Board pin definitions (from Setup212_LilyGo_T_Watch_S3.h)
#define BOARD_LCD_SCLK    18  // TFT_SCLK
#define BOARD_LCD_MOSI    13  // TFT_MOSI
#define BOARD_LCD_MISO    -1  // TFT_MISO (Not connected)
#define BOARD_LCD_DC      38  // TFT_DC
#define BOARD_LCD_CS      12  // TFT_CS
#define BOARD_LCD_RST     -1  // TFT_RST (Not connected)
#define BOARD_LCD_BL      45  // TFT_BL (LED back-light)

// Touch screen pins (from utilities.h)
#define BOARD_TOUCH_SDA   39
#define BOARD_TOUCH_SCL   40
#define BOARD_TOUCH_INT   16

// I2C pins (correct for T-Watch S3 - from utilities.h)
#define BOARD_I2C_SDA     10
#define BOARD_I2C_SCL     11

// Hardware components
XPowersAXP2101 PMU;
SensorBMA423 BMA;
SensorPCF8563 RTC;

// LovyanGFX Display Configuration (based on working test)
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
            // Rotation will be set after initialization
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

// Custom Interface Colors
#define COLOR_RED            0xF800     // Red (#FF0000)
#define COLOR_GREEN          0x07E0     // Green
#define COLOR_BLUE           0x001F     // Blue
#define COLOR_WHITE          0xFFFF     // White
#define COLOR_BLACK          0x0000     // Black
#define COLOR_YELLOW         0xFFE0     // Yellow
#define COLOR_ORANGE         0xFD20     // Orange
#define COLOR_PURPLE         0xF81F     // Purple
#define COLOR_CYAN           0x07FF     // Cyan
#define COLOR_DARK_GRAY      0x4208     // Dark Gray
#define COLOR_LIGHT_GRAY     0x8410     // Light Gray

// Custom Theme Colors
#define COLOR_CUSTOM_BG      0x0000     // Black background
#define COLOR_CUSTOM_TEXT    0xF800     // Red text (#FF0000)
#define COLOR_CUSTOM_ACCENT  0xFFE0     // Yellow accent
#define COLOR_CUSTOM_WARNING 0xF800     // Red warning
#define COLOR_CUSTOM_SUCCESS 0x07E0     // Green success
#define COLOR_CUSTOM_HIGHLIGHT 0xFFFF   // White highlight

// Anti-aliasing colors for smoother text
#define COLOR_TEXT_AA1       0x8000     // Darker red for anti-aliasing
#define COLOR_TEXT_AA2       0x4000     // Even darker red for anti-aliasing
#define COLOR_TEXT_AA3       0x2000     // Very dark red for anti-aliasing

#define COLOR_BACKGROUND     COLOR_CUSTOM_BG
#define COLOR_TEXT           COLOR_CUSTOM_TEXT

// WiFi network structure
struct WiFiNetwork {
    char ssid[32];
    char password[64];
    bool valid;
};

// Global variables
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

// Weather variables
bool weatherInitialized = false;
float weatherTemperature = 0.0;
String weatherCity = "São Paulo";
String weatherCountry = "BR";
String weatherApiKey = "";
String weatherDescription = "";
String weatherUnits = "metric";
unsigned long lastWeatherUpdate = 0;
unsigned long weatherUpdateInterval = 300000; // 5 minutes default
bool weatherConfigLoaded = false;

// WiFi image variables
bool wifiImageLoaded = false;
uint16_t* wifiImageBuffer = nullptr;
int wifiImageWidth = 0;
int wifiImageHeight = 0;


const unsigned long SLEEP_TIMEOUT = 10000;  // 10 seconds
const unsigned long DEEP_SLEEP_TIMEOUT = 30000;  // 30 seconds
bool deepSleepMode = false;
bool forceRedraw = false;  // Flag to force complete redraw

// Touch variables
bool touchInitialized = false;
uint16_t lastTouchX = 0, lastTouchY = 0;
bool touchDetected = false;

// NTP configuration
#define NTP_SERVER1           "pool.ntp.org"
#define NTP_SERVER2           "time.nist.gov"
#define GMT_OFFSET_SEC        0
#define DAY_LIGHT_OFFSET_SEC  (3600)

// Function to parse a line from the config file
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

// Function to load weather configuration from file
bool loadWeatherConfig() {
    Serial.println("Loading weather configuration...");
    
    if (!FFat.begin(true)) {
        Serial.println("FATFS initialization failed for weather config!");
        return false;
    }
    
    Serial.println("FATFS initialized successfully");
    
    if (!FFat.exists("/weather_config.ini")) {
        Serial.println("Weather config file not found!");
        return false;
    }
    
    Serial.println("Weather config file found!");
    
    File file = FFat.open("/weather_config.ini", "r");
    if (!file) {
        Serial.println("Failed to open weather config file!");
        return false;
    }
    
    String line;
    
    while (file.available()) {
        line = file.readStringUntil('\n');
        line.trim();
        
        // Skip comments and empty lines
        if (line.startsWith("#") || line.length() == 0) {
            continue;
        }
        
        // Parse API key
        if (line.startsWith("api_key=")) {
            weatherApiKey = getValue(line, '=', 1);
            Serial.println("Weather API key loaded");
        }
        
        // Parse city
        if (line.startsWith("city=")) {
            weatherCity = getValue(line, '=', 1);
            Serial.printf("Weather city: %s\n", weatherCity.c_str());
        }
        
        // Parse country
        if (line.startsWith("country=")) {
            weatherCountry = getValue(line, '=', 1);
            Serial.printf("Weather country: %s\n", weatherCountry.c_str());
        }
        
        // Parse update interval
        if (line.startsWith("update_interval=")) {
            String intervalStr = getValue(line, '=', 1);
            weatherUpdateInterval = intervalStr.toInt() * 1000; // Convert seconds to milliseconds
            Serial.printf("Weather update interval: %lu ms\n", weatherUpdateInterval);
        }
        
        // Parse units
        if (line.startsWith("units=")) {
            weatherUnits = getValue(line, '=', 1);
            Serial.printf("Weather units: %s\n", weatherUnits.c_str());
        }
    }
    
    file.close();
    
    // Check if API key is configured
    Serial.printf("API Key loaded: %s (length: %d)\n", weatherApiKey.c_str(), weatherApiKey.length());
    Serial.printf("City: %s, Country: %s, Units: %s\n", weatherCity.c_str(), weatherCountry.c_str(), weatherUnits.c_str());
    Serial.printf("Update interval: %lu ms\n", weatherUpdateInterval);
    
    if (weatherApiKey.length() > 0 && weatherApiKey != "YOUR_API_KEY_HERE") {
        weatherConfigLoaded = true;
        Serial.println("Weather configuration loaded successfully");
        return true;
    } else {
        Serial.println("Weather API key not configured!");
        return false;
    }
}

// Function to load WiFi configuration from file
bool loadWiFiConfig() {
    if (!FFat.begin(true)) {
        Serial.println("FATFS initialization failed!");
        return false;
    }
    
    if (!FFat.exists("/wifi_config.ini")) {
        Serial.println("WiFi config file not found!");
        return false;
    }
    
    File file = FFat.open("/wifi_config.ini", "r");
    if (!file) {
        Serial.println("Failed to open WiFi config file!");
        return false;
    }
    
    wifiNetworkCount = 0;
    String line;
    
    while (file.available() && wifiNetworkCount < 4) {
        line = file.readStringUntil('\n');
        line.trim();
        
        // Skip comments and empty lines
        if (line.startsWith("#") || line.length() == 0) {
            continue;
        }
        
        // Parse SSID lines
        if (line.startsWith("wifi") && line.indexOf("_ssid=") > 0) {
            String ssid = getValue(line, '=', 1);
            if (ssid.length() > 0) {
                strncpy(wifiNetworks[wifiNetworkCount].ssid, ssid.c_str(), 31);
                wifiNetworks[wifiNetworkCount].ssid[31] = '\0';
                wifiNetworks[wifiNetworkCount].valid = false; // Will be set to true when password is found
            }
        }
        
        // Parse password lines
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
    
    // Print loaded networks (without passwords for security)
    for (int i = 0; i < wifiNetworkCount; i++) {
        Serial.printf("Network %d: %s\n", i + 1, wifiNetworks[i].ssid);
    }
    
    return wifiNetworkCount > 0;
}

// Function to read battery information
void readBatteryInfo() {
    if (!pmuInitialized) return;
    
    // Read battery voltage
    batteryVoltage = PMU.getBattVoltage();
    
    // Read battery percentage
    batteryPercent = PMU.getBatteryPercent();
    
    // Check if battery is connected
    isBatteryConnected = PMU.isBatteryConnect();
    
    // Check if charging
    isCharging = PMU.isCharging();
    
    // Note: getBattCurrent() not available in XPowersLib
    // batteryCurrent = PMU.getBattCurrent();
    // batteryPower = batteryVoltage * batteryCurrent;
    
    // Debug output
    Serial.printf("Battery: %.2fV, %d%%, Charging: %s, Connected: %s\n", 
                 batteryVoltage, batteryPercent, 
                 isCharging ? "Yes" : "No", 
                 isBatteryConnected ? "Yes" : "No");
}

// Function to optimize battery usage based on battery level
void optimizeBatteryUsage() {
    if (!pmuInitialized) return;
    
    // More aggressive brightness control for better battery life
    if (batteryPercent <= 15) {
        // Very low battery - minimal brightness
        display.setBrightness(32);  // 12.5% brightness
        Serial.println("Very low battery: Minimal brightness");
    } else if (batteryPercent <= 30) {
        // Low battery - reduced brightness
        display.setBrightness(64);  // 25% brightness
        Serial.println("Low battery: Reduced brightness");
    } else if (batteryPercent <= 60) {
        // Medium battery - moderate brightness
        display.setBrightness(128); // 50% brightness
        Serial.println("Medium battery: Moderate brightness");
    } else {
        // High battery - good brightness (not full to save power)
        display.setBrightness(192); // 75% brightness
        Serial.println("High battery: Good brightness");
    }
    
    // Disable WiFi if battery is very low
    if (batteryPercent <= 10 && wifiConnected) {
        WiFi.disconnect();
        wifiConnected = false;
        Serial.println("Very low battery: WiFi disabled");
    }
    
    // Reduce CPU frequency for better battery life
    if (batteryPercent <= 20) {
        setCpuFrequencyMhz(80);  // Reduce to 80MHz for low battery
        Serial.println("Low battery: CPU frequency reduced to 80MHz");
    } else if (batteryPercent <= 50) {
        setCpuFrequencyMhz(160); // Reduce to 160MHz for medium battery
        Serial.println("Medium battery: CPU frequency reduced to 160MHz");
    } else {
        setCpuFrequencyMhz(240); // Full frequency for high battery
        Serial.println("High battery: Full CPU frequency");
    }
}

// Function to try connecting to WiFi networks
bool connectToWiFi() {
    if (wifiNetworkCount == 0) {
        Serial.println("No WiFi networks configured!");
        return false;
    }
    
    for (int i = 0; i < wifiNetworkCount; i++) {
        if (!wifiNetworks[i].valid) continue;
        
        Serial.printf("Trying to connect to: %s\n", wifiNetworks[i].ssid);
        
        WiFi.begin(wifiNetworks[i].ssid, wifiNetworks[i].password);
        int attempts = 0;
        
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("\nConnected to: %s\n", wifiNetworks[i].ssid);
            Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
            return true;
        } else {
            Serial.printf("\nFailed to connect to: %s\n", wifiNetworks[i].ssid);
            WiFi.disconnect();
        }
    }
    
    Serial.println("Failed to connect to any WiFi network!");
    return false;
}

void setup()
{
    Serial.begin(115200);
    Serial.println("T-Watch S3 Starting with TrueType fonts...");
    
    // Initialize display first with improved settings
    display.init();
    display.setRotation(2);  // Try rotation 2 for correct orientation
    display.setBrightness(255);
    
    // Configure display for better text rendering
    display.setColorDepth(16);  // Set color depth for better quality
    
    // Configure file system for LovyanGFX (commented out due to compatibility issues)
    // display.setFileStorage(FFat);
    
    // Set default TrueType font
    display.setFont(&fonts::Font2);
    
    // Clear screen and show startup message
    display.fillScreen(COLOR_BLACK);
    
    // Basic display test with TrueType text
    Serial.println("Testing TrueType fonts...");
    drawTrueTypeText("TRUETYPE TEST", 10, 50, 3, COLOR_TEXT);
    drawTrueTypeText("Fontes TrueType", 10, 80, 2, COLOR_TEXT);
    drawTrueTypeText("Qualidade Superior!", 10, 110, 1, COLOR_TEXT);
    
    delay(3000);
    
    // Clear screen
    display.fillScreen(COLOR_BLACK);
    
    // Initialize I2C
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
    Serial.println("I2C initialized");
    
    // Initialize touch screen
    Wire1.begin(BOARD_TOUCH_SDA, BOARD_TOUCH_SCL);
    Serial.println("Touch I2C initialized");
    
    // Test touch screen
    if (display.getTouch(&lastTouchX, &lastTouchY)) {
        touchInitialized = true;
        Serial.println("Touch screen initialized successfully");
    } else {
        Serial.println("Touch screen initialization failed, will retry");
    }
    
    // Initialize PMU
    if (PMU.begin(Wire, AXP2101_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
        Serial.println("PMU initialized");
        pmuInitialized = true;
        
        // Set up PMU
        PMU.setDC3Voltage(3000);  // Set DC3 to 3.0V
        PMU.enableDC3();
        PMU.setALDO2Voltage(3300); // Set ALDO2 to 3.3V
        PMU.enableALDO2();
        PMU.setALDO3Voltage(2800); // Set ALDO3 to 2.8V
        PMU.enableALDO3();
        PMU.setALDO4Voltage(2800); // Set ALDO4 to 2.8V
        PMU.enableALDO4();
        
        // Read initial battery info
        readBatteryInfo();
        
        display.setCursor(20, 140);
        display.println("PMU: OK");
    } else {
        Serial.println("PMU initialization failed on all addresses");
        display.setCursor(20, 140);
        display.println("PMU: INIT FAIL");
    }
    
    // Initialize BMA423
    if (BMA.begin(Wire, BMA423_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
        Serial.println("BMA423 initialized");
        BMA.configAccelerometer();
        BMA.enableAccelerometer();
        BMA.enablePedometer();
        display.setCursor(20, 170);
        display.println("BMA: OK");
    } else {
        Serial.println("BMA423 initialization failed!");
        display.setCursor(20, 170);
        display.println("BMA: FAIL");
    }
    

    
    // Initialize RTC
    if (RTC.begin(Wire, PCF8563_SLAVE_ADDRESS, BOARD_I2C_SDA, BOARD_I2C_SCL)) {
        Serial.println("RTC initialized");
        display.setCursor(20, 200);
        display.println("RTC: OK");
    } else {
        Serial.println("RTC initialization failed!");
        display.setCursor(20, 200);
        display.println("RTC: FAIL");
    }
    
    delay(2000);
    
    // Load WiFi configuration and try to connect
    Serial.println("Loading WiFi configuration...");
    display.fillScreen(COLOR_BLACK);
    display.setCursor(20, 80);
    display.println("Loading WiFi...");
    
    if (loadWiFiConfig()) {
        Serial.println("WiFi config loaded successfully");
        display.setCursor(20, 110);
        display.println("WiFi Config: OK");
        
        delay(1000);
        
        // Try to connect to WiFi networks
        Serial.println("Connecting to WiFi...");
        display.fillScreen(COLOR_BLACK);
        display.setCursor(20, 80);
        display.println("Connecting WiFi...");
        
        if (connectToWiFi()) {
            Serial.println("WiFi connected!");
            wifiConnected = true;
            display.setCursor(20, 110);
            display.println("WiFi: CONNECTED");
            
            // Sync time
            configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);
            Serial.println("Time sync configured");
            
            // Wait for time sync
            int timeSyncAttempts = 0;
            while (!getLocalTime(&timeinfo) && timeSyncAttempts < 10) {
                delay(1000);
                timeSyncAttempts++;
            }
            
            if (getLocalTime(&timeinfo)) {
                Serial.println("Time synchronized!");
                display.setCursor(20, 140);
                display.println("Time: SYNCED");
            } else {
                Serial.println("Time sync failed!");
                display.setCursor(20, 140);
                display.println("Time: FAIL");
            }
        } else {
            Serial.println("WiFi connection failed!");
            display.setCursor(20, 110);
            display.println("WiFi: FAILED");
        }
    } else {
        Serial.println("Failed to load WiFi configuration!");
        display.setCursor(20, 110);
        display.println("WiFi Config: FAIL");
    }
    
    delay(2000);
    
    // Load weather configuration
    Serial.println("Loading weather configuration...");
    display.fillScreen(COLOR_BLACK);
    display.setCursor(20, 80);
    display.println("Loading Weather Config...");
    
    if (loadWeatherConfig()) {
        Serial.println("Weather configuration loaded successfully");
        display.setCursor(20, 110);
        display.println("Weather Config: OK");
    } else {
        Serial.println("Failed to load weather configuration!");
        display.setCursor(20, 110);
        display.println("Weather Config: FAIL");
    }
    
    delay(1000);
    
    // Load WiFi image
    loadWiFiImage();
    
    // Initialize sleep mode
    lastActivity = millis();
    Serial.println("Sleep mode initialized");
    
    Serial.println("Setup completed!");
}

void drawStepsIcon(int x, int y) {
    // Draw pixelated steps/footprint icon
    display.fillRect(x, y + 6, 3, 3, COLOR_CUSTOM_ACCENT);      // Footprint
    display.fillRect(x + 3, y + 3, 3, 3, COLOR_CUSTOM_ACCENT);
    display.fillRect(x + 6, y, 3, 3, COLOR_CUSTOM_ACCENT);
    display.fillRect(x + 9, y + 3, 3, 3, COLOR_CUSTOM_ACCENT);
    display.fillRect(x + 12, y + 6, 3, 3, COLOR_CUSTOM_ACCENT);
}

void drawTimeSeparator(int x, int y) {
    // Draw animated time separator (blinking colon)
    static bool separatorVisible = true;
    static unsigned long lastBlink = 0;
    
    if (millis() - lastBlink > 500) {
        separatorVisible = !separatorVisible;
        lastBlink = millis();
    }
    
    if (separatorVisible) {
        display.fillRect(x, y, 4, 4, COLOR_CUSTOM_ACCENT);
        display.fillRect(x, y + 10, 4, 4, COLOR_CUSTOM_ACCENT);
    }
}

void formatTimeString(char* timeStr, int hour, int minute) {
    // Format: 24-hour with leading zeros (no seconds)
    sprintf(timeStr, "%02d:%02d", hour, minute);
}



void drawWiFiIcon(int x, int y, bool isConnected) {
    if (wifiImageLoaded) {
        // Draw the appropriate PNG image based on connection status
        Serial.printf("Drawing PNG WiFi icon at (%d,%d), connected: %s\n", 
                     x, y, isConnected ? "YES" : "NO");
        
        // Adjust position to center the PNG (18x15 pixels)
        int pngX = x - 9;  // Center horizontally (18/2 = 9)
        int pngY = y - 7;  // Center vertically (15/2 = 7)
        
        bool success = false;
        
        if (isConnected) {
            // Draw green WiFi icon
            success = display.drawPng(data_assets_wifi_green_png, data_assets_wifi_green_png_len, pngX, pngY);
        } else {
            // Draw red WiFi icon (original)
            success = display.drawPng(data_assets_wifi_small_png, data_assets_wifi_small_png_len, pngX, pngY);
        }
        
        if (success) {
            Serial.printf("PNG WiFi icon drawn successfully (%s) at (%d,%d)!\n", 
                         isConnected ? "GREEN" : "RED", pngX, pngY);
            return;
        } else {
            Serial.println("Failed to draw PNG, using vector fallback");
        }
    }
    
    // Use vector graphics as fallback
    drawVectorWiFiIcon(x, y, isConnected);
}

void drawVectorWiFiIcon(int x, int y, bool isConnected) {
    uint16_t iconColor = isConnected ? TFT_GREEN : TFT_RED;
    
    // Draw a more sophisticated WiFi icon using vector graphics
    int centerX = x;
    int centerY = y;
    
    // Draw WiFi signal arcs (semi-circles) - CORRECTED ORIENTATION
    for (int i = 0; i < 3; i++) {
        int radius = 8 + i * 4;  // 8, 12, 16
        int startAngle = 225;    // Start at 225 degrees (bottom-left)
        int endAngle = 315;      // End at 315 degrees (bottom-right)
        
        // Draw arc using multiple line segments
        for (int angle = startAngle; angle <= endAngle; angle += 5) {
            int x1 = centerX + (radius - 1) * cos(angle * PI / 180);
            int y1 = centerY + (radius - 1) * sin(angle * PI / 180);
            int x2 = centerX + (radius + 1) * cos(angle * PI / 180);
            int y2 = centerY + (radius + 1) * sin(angle * PI / 180);
            
            display.drawLine(x1, y1, x2, y2, iconColor);
        }
    }
    
    // Draw center dot
    display.fillCircle(centerX, centerY, 2, iconColor);
    
    // Draw connection status indicator
    if (isConnected) {
        // Green dot for connected
        display.fillCircle(centerX, centerY - 12, 2, TFT_GREEN);
    } else {
        // Red X for disconnected
        display.drawLine(centerX - 3, centerY - 9, centerX + 3, centerY - 15, TFT_RED);
        display.drawLine(centerX + 3, centerY - 9, centerX - 3, centerY - 15, TFT_RED);
    }
    
    Serial.printf("Drew vector WiFi icon at (%d,%d), color: %s\n", 
                 x, y, isConnected ? "GREEN" : "RED");
}

void enterSleepMode() {
    if (!displaySleep) {
        Serial.println("Entering sleep mode...");
        displaySleep = true;
        
        // Turn off display completely
        display.setBrightness(0);  // Turn off backlight
        display.fillScreen(COLOR_BACKGROUND);  // Clear screen
        
        // Disable WiFi to save power
        if (wifiConnected) {
            WiFi.disconnect();
            wifiConnected = false;
            Serial.println("WiFi disabled for sleep mode");
        }
        
        // Reduce CPU frequency for sleep mode
        setCpuFrequencyMhz(80);
        Serial.println("CPU frequency reduced to 80MHz for sleep");
    }
}

void exitSleepMode() {
    if (displaySleep) {
        Serial.println("Exiting sleep mode...");
        displaySleep = false;
        deepSleepMode = false;
        lastActivity = millis();
        
        // Restore brightness based on battery level
        if (batteryPercent <= 20) {
            display.setBrightness(64);  // 25% brightness
        } else if (batteryPercent <= 50) {
            display.setBrightness(128); // 50% brightness
        } else {
            display.setBrightness(255); // 100% brightness
        }
        
        // Force complete redraw
        needsFullRedraw = true;
    }
}

void checkActivity() {
    // Check for accelerometer movement
    if (BMA.isPedometer()) {
        uint32_t currentSteps = BMA.getPedometerCounter();
        if (currentSteps != lastStepCount) {
            lastActivity = millis();
            lastStepCount = currentSteps;
            if (displaySleep) {
                exitSleepMode();
            }
        }
    }
    
    // Check for accelerometer movement (more sensitive)
    static int16_t lastAccelX = 0, lastAccelY = 0, lastAccelZ = 0;
    int16_t rawBuffer[3];
    
    // Read accelerometer data
    if (BMA.getAccelRaw(rawBuffer)) {
        int16_t accelX = rawBuffer[0];
        int16_t accelY = rawBuffer[1];
        int16_t accelZ = rawBuffer[2];
        
        // Check if there's significant movement
        int32_t movement = abs(accelX - lastAccelX) + abs(accelY - lastAccelY) + abs(accelZ - lastAccelZ);
        if (movement > 100) {  // Threshold for movement detection
            lastActivity = millis();
            if (displaySleep) {
                exitSleepMode();
            }
        }
        
        lastAccelX = accelX;
        lastAccelY = accelY;
        lastAccelZ = accelZ;
    }
    
    // Check for touch screen activity
    uint16_t touchX, touchY;
    if (display.getTouch(&touchX, &touchY)) {
        // Only trigger if touch position changed significantly
        if (abs(touchX - lastTouchX) > 5 || abs(touchY - lastTouchY) > 5) {
            lastActivity = millis();
            lastTouchX = touchX;
            lastTouchY = touchY;
            touchDetected = true;
            Serial.printf("Touch detected at (%d, %d)\n", touchX, touchY);
            
            if (displaySleep) {
                exitSleepMode();
            }
        }
    } else {
        touchDetected = false;
    }
    
    // Check for button press (if available)
    // Note: T-Watch S3 has buttons, but we need to implement them
}

void manageSleepMode() {
    unsigned long currentTime = millis();
    
    // Check if we should enter sleep mode
    if (!displaySleep && (currentTime - lastActivity > SLEEP_TIMEOUT)) {
        enterSleepMode();
    }
    
    // Check if we should enter deep sleep mode
    if (displaySleep && !deepSleepMode && (currentTime - lastActivity > DEEP_SLEEP_TIMEOUT)) {
        Serial.println("Entering deep sleep mode...");
        deepSleepMode = true;
        
        // Disable WiFi to save power
        if (wifiConnected) {
            WiFi.disconnect();
            wifiConnected = false;
            Serial.println("WiFi disabled for deep sleep");
        }
        
        // Reduce CPU frequency
        setCpuFrequencyMhz(80);  // Reduce from 240MHz to 80MHz
        Serial.println("CPU frequency reduced to 80MHz");
    }
    
    // Check for activity to wake up
    checkActivity();
}

// Function to load WiFi image from assets
bool loadWiFiImage() {
    if (wifiImageLoaded) return true;
    
    Serial.println("Loading WiFi image from embedded data...");
    
    // Test PNG loading using embedded array data
    bool success = display.drawPng(data_assets_wifi_small_png, data_assets_wifi_small_png_len, -100, -100);
    
    if (success) {
        wifiImageLoaded = true;
        Serial.printf("WiFi PNG image loaded successfully! Size: %d bytes\n", data_assets_wifi_small_png_len);
        return true;
    } else {
        Serial.println("PNG loading failed, will use vector icon");
    }
    
    Serial.println("Using fallback vector WiFi icon");
    return false;
}

// Function to get weather data from OpenWeatherMap API
void getWeatherData() {
    if (!wifiConnected) {
        Serial.println("WiFi not connected, cannot get weather data");
        return;
    }
    
    if (!weatherConfigLoaded) {
        Serial.println("Weather configuration not loaded, cannot get weather data");
        return;
    }
    
    // Check if it's time to update weather
    if (millis() - lastWeatherUpdate < weatherUpdateInterval) {
        Serial.printf("Weather update not due yet. Next update in %lu ms\n", 
                     weatherUpdateInterval - (millis() - lastWeatherUpdate));
        return;
    }
    
    Serial.println("Getting weather data...");
    Serial.printf("City: %s, Country: %s, Units: %s\n", 
                 weatherCity.c_str(), weatherCountry.c_str(), weatherUnits.c_str());
    
    // Create HTTP client
    HTTPClient http;
    
    // Build location string (city,country)
    String location = weatherCity;
    if (weatherCountry.length() > 0) {
        location += "," + weatherCountry;
    }
    
    // OpenWeatherMap API URL with configuration
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + location + "&appid=" + weatherApiKey + "&units=" + weatherUnits;
    
    Serial.printf("Weather API URL: %s\n", url.c_str());
    
    http.begin(url);
    http.setTimeout(10000); // 10 second timeout
    
    int httpCode = http.GET();
    Serial.printf("HTTP response code: %d\n", httpCode);
    
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.printf("Weather data received, payload length: %d\n", payload.length());
        Serial.printf("Payload preview: %s\n", payload.substring(0, 200).c_str());
        
        // Parse JSON response (simplified parsing)
        int tempIndex = payload.indexOf("\"temp\":");
        if (tempIndex > 0) {
            int tempStart = payload.indexOf(":", tempIndex) + 1;
            int tempEnd = payload.indexOf(",", tempStart);
            if (tempEnd == -1) tempEnd = payload.indexOf("}", tempStart);
            
            String tempStr = payload.substring(tempStart, tempEnd);
            weatherTemperature = tempStr.toFloat();
            
            Serial.printf("Weather temperature: %.1f°C\n", weatherTemperature);
            weatherInitialized = true;
        } else {
            Serial.println("Temperature not found in response");
            Serial.printf("Full response: %s\n", payload.c_str());
        }
        
        // Parse weather description
        int descIndex = payload.indexOf("\"description\":\"");
        if (descIndex > 0) {
            int descStart = descIndex + 15;
            int descEnd = payload.indexOf("\"", descStart);
            weatherDescription = payload.substring(descStart, descEnd);
            Serial.printf("Weather description: %s\n", weatherDescription.c_str());
        }
        
    } else {
        Serial.printf("HTTP request failed, error: %d\n", httpCode);
        String errorPayload = http.getString();
        Serial.printf("Error response: %s\n", errorPayload.c_str());
    }
    
    http.end();
    lastWeatherUpdate = millis();
    Serial.printf("Weather update completed. Next update in %lu ms\n", weatherUpdateInterval);
}



// Function to draw text with TrueType fonts using LovyanGFX native fonts
void drawTrueTypeText(const char* text, int x, int y, int fontSize, uint16_t color) {
    display.setTextColor(color);
    
    // Use LovyanGFX native fonts with TrueType-like quality
    switch(fontSize) {
        case 1: // Small text (status, battery, etc.) - 6x8 pixels
            display.setFont(&fonts::Font0);
            display.setTextSize(1);
            break;
        case 2: // Medium text (date, day, etc.) - 12x16 pixels
            display.setFont(&fonts::Font2);
            display.setTextSize(1);
            break;
        case 3: // Large text (time) - 16x24 pixels
            display.setFont(&fonts::Font4);
            display.setTextSize(1);
            break;
        case 4: // Extra large text (main time) - 20x32 pixels
            display.setFont(&fonts::Font6);
            display.setTextSize(1);
            break;
        case 5: // Super large text (main time - 30% bigger) - 26x42 pixels
            display.setFont(&fonts::Font6);
            display.setTextSize(1.3); // 30% bigger
            break;
        default:
            display.setFont(&fonts::Font2);
            display.setTextSize(1);
            break;
    }
    
    display.setCursor(x, y);
    display.print(text);
}

// Function to get text width with TrueType font
int getTrueTypeTextWidth(const char* text, int fontSize) {
    // Select font temporarily to measure
    switch(fontSize) {
        case 1:
            display.setFont(&fonts::Font0);
            display.setTextSize(1);
            break;
        case 2:
            display.setFont(&fonts::Font2);
            display.setTextSize(1);
            break;
        case 3:
            display.setFont(&fonts::Font4);
            display.setTextSize(1);
            break;
        case 4:
            display.setFont(&fonts::Font6);
            display.setTextSize(1);
            break;
        case 5:
            display.setFont(&fonts::Font6);
            display.setTextSize(1.3);
            break;
        default:
            display.setFont(&fonts::Font2);
            display.setTextSize(1);
            break;
    }
    
    return display.textWidth(text);
}

// Function to get font height with TrueType font
int getTrueTypeFontHeight(int fontSize) {
    // Select font temporarily to measure
    switch(fontSize) {
        case 1:
            display.setFont(&fonts::Font0);
            display.setTextSize(1);
            break;
        case 2:
            display.setFont(&fonts::Font2);
            display.setTextSize(1);
            break;
        case 3:
            display.setFont(&fonts::Font4);
            display.setTextSize(1);
            break;
        case 4:
            display.setFont(&fonts::Font6);
            display.setTextSize(1);
            break;
        case 5:
            display.setFont(&fonts::Font6);
            display.setTextSize(1.3);
            break;
        default:
            display.setFont(&fonts::Font2);
            display.setTextSize(1);
            break;
    }
    
    return display.fontHeight();
}

// Updated drawCustomInterface function with TrueType fonts
void drawCustomInterface() {
    if (displaySleep) return;
    
    // Get current time
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return;
    }
    
    // Format time string - Always 24h format with leading zeros
    char timeStr[20];
    sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    
    // Format date string
    char dateStr[20];
    sprintf(dateStr, "%02d/%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1);
    
    // Format day string
    char dayStr[20];
    const char* days[] = {"DOM", "SEG", "TER", "QUA", "QUI", "SEX", "SAB"};
    strcpy(dayStr, days[timeinfo.tm_wday]);
    
    // Format battery string
    char batteryStr[16];
    sprintf(batteryStr, "%d%%", batteryPercent);
    
    // Format WiFi string
    char wifiStr[16];
    strcpy(wifiStr, wifiConnected ? "WiFi ON" : "WiFi OFF");
    
    // Check if we need to redraw
    bool needsRedraw = needsFullRedraw || 
                      strcmp(timeStr, lastTimeStr) != 0 ||
                      strcmp(dateStr, lastDateStr) != 0 ||
                      strcmp(dayStr, lastDayStr) != 0 ||
                      batteryPercent != lastBatteryPercent ||
                      isCharging != lastChargingState ||
                      stepCounter != lastStepCount ||
                      wifiConnected != lastWifiState;
    
    if (!needsRedraw) return;
    
    // Clear screen if full redraw needed
    if (needsFullRedraw) {
        display.fillScreen(COLOR_BACKGROUND);
        needsFullRedraw = false;
    }
    
    // Calculate positions with TrueType fonts
    int screenWidth = display.width();
    int screenHeight = display.height();
    
    // Time position (center, extra large font - 30% bigger)
    int timeWidth = getTrueTypeTextWidth(timeStr, 5); // Using larger font
    int timeX = (screenWidth - timeWidth) / 2;
    int timeY = 70; // Adjusted for larger font
    
    // Date position (center, medium font)
    int dateWidth = getTrueTypeTextWidth(dateStr, 2);
    int dateX = (screenWidth - dateWidth) / 2;
    int dateY = timeY + getTrueTypeFontHeight(4) + 10;
    
    // Day position (center, medium font)
    int dayWidth = getTrueTypeTextWidth(dayStr, 2);
    int dayX = (screenWidth - dayWidth) / 2;
    int dayY = dateY + getTrueTypeFontHeight(2) + 5;
    
    // Battery position (top right, small font)
    int batteryWidth = getTrueTypeTextWidth(batteryStr, 1);
    int batteryX = screenWidth - batteryWidth - 10;
    int batteryY = 10;
    
    // WiFi position (top left, icon)
    int wifiX = 25;
    int wifiY = 25;
    
    // Steps position (bottom, small font)
    char stepsStr[16];
    sprintf(stepsStr, "Steps: %lu", stepCounter);
    int stepsWidth = getTrueTypeTextWidth(stepsStr, 1);
    int stepsX = (screenWidth - stepsWidth) / 2;
    int stepsY = screenHeight - getTrueTypeFontHeight(1) - 10;
    
    // Weather Temperature position (bottom right, small font)
    char tempStr[16];
    if (weatherConfigLoaded && weatherInitialized) {
        sprintf(tempStr, "%.1f°C", weatherTemperature);
    } else if (weatherConfigLoaded) {
        sprintf(tempStr, "N/A");
    } else {
        sprintf(tempStr, "NO API");
    }
    int tempWidth = getTrueTypeTextWidth(tempStr, 1);
    int tempX = screenWidth - tempWidth - 10;
    int tempY = screenHeight - getTrueTypeFontHeight(1) - 10;
    
    // Clear old text areas if values changed
    if (strcmp(timeStr, lastTimeStr) != 0) {
        // Clear time area
        display.fillRect(timeX - 5, timeY - 5, timeWidth + 10, getTrueTypeFontHeight(4) + 10, COLOR_BACKGROUND);
    }
    
    if (strcmp(dateStr, lastDateStr) != 0) {
        // Clear date area
        display.fillRect(dateX - 5, dateY - 5, dateWidth + 10, getTrueTypeFontHeight(2) + 10, COLOR_BACKGROUND);
    }
    
    if (strcmp(dayStr, lastDayStr) != 0) {
        // Clear day area
        display.fillRect(dayX - 5, dayY - 5, dayWidth + 10, getTrueTypeFontHeight(2) + 10, COLOR_BACKGROUND);
    }
    
    if (batteryPercent != lastBatteryPercent || isCharging != lastChargingState) {
        // Clear battery area
        display.fillRect(batteryX - 5, batteryY - 5, batteryWidth + 10, getTrueTypeFontHeight(1) + 10, COLOR_BACKGROUND);
    }
    
    if (wifiConnected != lastWifiState) {
        // Clear WiFi area (larger area for icon)
        display.fillRect(wifiX - 15, wifiY - 15, 30, 30, COLOR_BACKGROUND);
    }
    
    if (stepCounter != lastStepCount) {
        // Clear steps area
        display.fillRect(stepsX - 5, stepsY - 5, stepsWidth + 10, getTrueTypeFontHeight(1) + 10, COLOR_BACKGROUND);
    }
    
    // Draw new text with TrueType fonts
    drawTrueTypeText(timeStr, timeX, timeY, 4, COLOR_TEXT);
    drawTrueTypeText(dateStr, dateX, dateY, 2, COLOR_TEXT);
    drawTrueTypeText(dayStr, dayX, dayY, 2, COLOR_TEXT);
    
    // Battery with color coding
    uint16_t batteryColor = COLOR_TEXT;
    if (batteryPercent <= 20) batteryColor = COLOR_RED;
    else if (batteryPercent <= 50) batteryColor = COLOR_ORANGE;
    else batteryColor = COLOR_GREEN;
    
    drawTrueTypeText(batteryStr, batteryX, batteryY, 1, batteryColor);
    
    // WiFi icon with color coding
    drawWiFiIcon(wifiX, wifiY, wifiConnected);
    
    // Steps
    drawTrueTypeText(stepsStr, stepsX, stepsY, 1, COLOR_TEXT);
    
    // Temperature
    drawTrueTypeText(tempStr, tempX, tempY, 1, COLOR_TEXT);
    
    // Update last values
    strcpy(lastTimeStr, timeStr);
    strcpy(lastDateStr, dateStr);
    strcpy(lastDayStr, dayStr);
    lastBatteryPercent = batteryPercent;
    lastChargingState = isCharging;
    lastStepCount = stepCounter;
    lastWifiState = wifiConnected;
    
    // Debug info
    Serial.printf("Time: %s, Date: %s, Day: %s, Battery: %d%%, WiFi: %s, Steps: %lu, Weather: %.1f°C\n", 
                  timeStr, dateStr, dayStr, batteryPercent, wifiStr, stepCounter, weatherTemperature);
}

void loop()
{
    // Manage sleep mode
    manageSleepMode();
    
    // Only update display if not in sleep mode
    if (!displaySleep) {
        // Read battery information every 10 seconds (reduced from 5s for battery life)
        static unsigned long lastBatteryRead = 0;
        if (millis() - lastBatteryRead > 10000) {
            readBatteryInfo();
            getWeatherData();  // Get weather data
            optimizeBatteryUsage();  // Apply battery optimizations
            lastBatteryRead = millis();
        }
        
        // Only redraw if something changed or forced redraw
        bool needsUpdate = needsFullRedraw || 
                          (timeinfo.tm_min != lastMinute) || 
                          (timeinfo.tm_hour != lastHour) ||
                          (timeinfo.tm_mday != lastDay) ||
                          (timeinfo.tm_mon != lastMonth) ||
                          (wifiConnected != lastWifiState) ||
                          (batteryPercent != lastBatteryPercent) ||
                          touchDetected;  // Redraw on touch
        
        if (needsUpdate) {
            drawCustomInterface();  // Use the optimized custom interface
        }
        
        delay(2000);  // Increased delay to 2 seconds for better battery life
    } else {
        // In sleep mode, just check for activity
        delay(500);  // Increased delay for better battery life
    }
} 