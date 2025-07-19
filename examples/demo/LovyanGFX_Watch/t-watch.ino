#include <LovyanGFX.hpp>
#include <WiFi.h>
#include <Wire.h>
#include <XPowersLib.h>
#include <SensorBMA423.hpp>
#include <SensorPCF8563.hpp>
#include "esp_sntp.h"
#include <FFat.h>
#include <FS.h>

// Board pin definitions (from Setup212_LilyGo_T_Watch_S3.h)
#define BOARD_LCD_SCLK    18  // TFT_SCLK
#define BOARD_LCD_MOSI    13  // TFT_MOSI
#define BOARD_LCD_MISO    -1  // TFT_MISO (Not connected)
#define BOARD_LCD_DC      38  // TFT_DC
#define BOARD_LCD_CS      12  // TFT_CS
#define BOARD_LCD_RST     -1  // TFT_RST (Not connected)
#define BOARD_LCD_BL      45  // TFT_BL (LED back-light)

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

        setPanel(&_panel_instance);
    }
};

LGFX display;

// Colors (adjusted for ST7789)
#define COLOR_RED            0x00FFFF   // Cyan (will be inverted to red)
#define COLOR_GREEN          0xFF00FF   // Magenta (will be inverted to green)
#define COLOR_BLUE           0xFFFF00   // Yellow (will be inverted to blue)
#define COLOR_WHITE          0x000000   // Black (will be inverted to white)
#define COLOR_BLACK          0xFFFFFF   // White (will be inverted to black)
#define COLOR_BACKGROUND     COLOR_BLACK
#define COLOR_TEXT           COLOR_WHITE

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
    
    // Adjust screen brightness based on battery level
    if (batteryPercent <= 20) {
        // Low battery - reduce brightness
        display.setBrightness(64);  // 25% brightness
        Serial.println("Low battery: Reduced brightness");
    } else if (batteryPercent <= 50) {
        // Medium battery - moderate brightness
        display.setBrightness(128); // 50% brightness
        Serial.println("Medium battery: Moderate brightness");
    } else {
        // High battery - full brightness
        display.setBrightness(255); // 100% brightness
        Serial.println("High battery: Full brightness");
    }
    
    // Disable WiFi if battery is very low (optional)
    if (batteryPercent <= 10 && wifiConnected) {
        WiFi.disconnect();
        wifiConnected = false;
        Serial.println("Very low battery: WiFi disabled");
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
    delay(2000);  // Wait for serial to be ready
    Serial.println("=== T-Watch S3 Working Firmware ===");
    Serial.flush();
    delay(100);
    
    // Initialize display FIRST (proven to work)
    Serial.println("Initializing display...");
    Serial.flush();
    display.init();
    Serial.println("Display initialized");
    
    display.setRotation(2);  // Fix inverted display
    display.setBrightness(128);
    Serial.println("Display configured");
    
    // Show initial screen
    display.fillScreen(COLOR_BACKGROUND);
    display.setTextColor(COLOR_TEXT);
    display.setTextSize(2);
    display.setCursor(20, 80);
    display.println("T-Watch S3");
    display.setCursor(20, 110);
    display.println("Starting...");
    
    // Initialize I2C with correct T-Watch S3 pins
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
    Serial.println("I2C initialized with correct pins (10, 11)");
    
    // Initialize PMU - Enhanced I2C diagnosis
    Serial.println("=== I2C Diagnosis ===");
    
    // Scan I2C bus
    Serial.println("Scanning I2C bus...");
    int deviceCount = 0;
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            deviceCount++;
        }
    }
    
    if (deviceCount == 0) {
        Serial.println("No I2C devices found!");
        display.setCursor(20, 140);
        display.println("I2C: NO DEVICES");
    } else {
        Serial.printf("Found %d I2C device(s)\n", deviceCount);
        
        // Try to initialize PMU at common addresses
        byte pmuAddresses[] = {0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B};
        bool pmuFound = false;
        
        for (int i = 0; i < 8; i++) {
            Wire.beginTransmission(pmuAddresses[i]);
            byte error = Wire.endTransmission();
            if (error == 0) {
                Serial.printf("Trying PMU at address 0x%02X\n", pmuAddresses[i]);
                if (PMU.begin(Wire, pmuAddresses[i], BOARD_I2C_SDA, BOARD_I2C_SCL)) {
                    Serial.println("PMU initialized successfully!");
                    display.setCursor(20, 140);
                    display.println("PMU: OK");
                    pmuFound = true;
                    pmuInitialized = true;
                    break;
                } else {
                    Serial.printf("PMU init failed at 0x%02X\n", pmuAddresses[i]);
                }
            }
        }
        
        if (!pmuFound) {
            Serial.println("PMU initialization failed on all addresses");
            display.setCursor(20, 140);
            display.println("PMU: INIT FAIL");
        }
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
    display.fillScreen(COLOR_BACKGROUND);
    display.setCursor(20, 80);
    display.println("Loading WiFi...");
    
    if (loadWiFiConfig()) {
        Serial.println("WiFi config loaded successfully");
        display.setCursor(20, 110);
        display.println("WiFi Config: OK");
        
        delay(1000);
        
        // Try to connect to WiFi networks
        Serial.println("Connecting to WiFi...");
        display.fillScreen(COLOR_BACKGROUND);
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
    
    Serial.println("Setup completed!");
}

void drawClock()
{
    display.fillScreen(COLOR_BACKGROUND);
    
    // Get current time
    if (!getLocalTime(&timeinfo)) {
        display.setTextColor(COLOR_RED);
        display.setTextSize(2);
        display.setCursor(20, 100);
        display.println("NO TIME");
        return;
    }
    
    // Draw time
    display.setTextColor(COLOR_TEXT);
    display.setTextSize(4);
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    display.setCursor(40, 80);
    display.println(timeStr);
    
    // Draw seconds
    display.setTextSize(2);
    sprintf(timeStr, "%02d", timeinfo.tm_sec);
    display.setCursor(180, 90);
    display.println(timeStr);
    
    // Draw date
    display.setTextSize(2);
    char dateStr[20];
    sprintf(dateStr, "%02d/%02d/%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);
    display.setCursor(20, 140);
    display.println(dateStr);
    
    // Draw battery status
    display.setTextSize(2);
    if (pmuInitialized && isBatteryConnected) {
        char batteryStr[50];
        
        // Show battery percentage and voltage
        sprintf(batteryStr, "BAT: %d%% %.2fV", batteryPercent, batteryVoltage);
        display.setCursor(20, 170);
        display.println(batteryStr);
        
        // Show charging status
        if (isCharging) {
            display.setTextColor(COLOR_GREEN);
            display.setCursor(20, 190);
            display.println("CHARGING");
            display.setTextColor(COLOR_TEXT);
        }
    } else {
        display.setTextColor(COLOR_RED);
        display.setCursor(20, 170);
        display.println("BAT: N/A");
        display.setTextColor(COLOR_TEXT);
    }
    
    // Draw steps
    if (BMA.isPedometer()) {
        stepCounter = BMA.getPedometerCounter();
        sprintf(dateStr, "Steps: %lu", stepCounter);
        display.setCursor(20, 200);
        display.println(dateStr);
    }
}

void loop()
{
    // Read battery information every 5 seconds
    static unsigned long lastBatteryRead = 0;
    if (millis() - lastBatteryRead > 5000) {
        readBatteryInfo();
        optimizeBatteryUsage();  // Apply battery optimizations
        lastBatteryRead = millis();
    }
    
    drawClock();
    delay(1000);
} 