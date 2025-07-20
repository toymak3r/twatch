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

// Sleep mode variables
bool displaySleep = false;
unsigned long lastActivity = 0;

// Time format setting (true = 12h, false = 24h)
bool use12HourFormat = true;
const unsigned long SLEEP_TIMEOUT = 10000;  // 10 seconds
const unsigned long DEEP_SLEEP_TIMEOUT = 30000;  // 30 seconds
bool deepSleepMode = false;
bool forceRedraw = false;  // Flag to force complete redraw

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
    
    // Setup custom NexaRustSlab font
    setupNexaRustSlabFont();
    Serial.println("Display configured with NexaRustSlab font");
    
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
    
    // Initialize sleep mode
    lastActivity = millis();
    Serial.println("Sleep mode initialized");
    
    Serial.println("Setup completed!");
}

// Removed Pip-Boy border, header, and menu bar functions as they're not needed for the custom interface

// Removed Pip-Boy battery icon function as it's not needed for the custom interface

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
    if (use12HourFormat) {
        // Format: 12-hour with AM/PM (no seconds)
        int hour12 = hour % 12;
        if (hour12 == 0) hour12 = 12;
        const char* ampm = (hour >= 12) ? "PM" : "AM";
        sprintf(timeStr, "%02d:%02d %s", hour12, minute, ampm);
    } else {
        // Format: 24-hour (no seconds)
        sprintf(timeStr, "%02d:%02d", hour, minute);
    }
}

void setupNexaRustSlabFont() {
    // Use a built-in font that looks similar to NexaRustSlab
    // For now, use the default font with larger text sizes to simulate the style
    display.setFont(&fonts::Font0);
    Serial.println("Using default font with NexaRustSlab-style sizing");
}

void drawCustomWiFiIcon(int x, int y) {
    // Draw custom WiFi icon in red
    int iconSize = 16;
    
    // WiFi signal lines (custom style)
    display.fillRect(x - 4, y + 8, 2, 2, COLOR_CUSTOM_TEXT);   // Bottom dot
    display.fillRect(x - 2, y + 6, 2, 2, COLOR_CUSTOM_TEXT);   // Middle dot
    display.fillRect(x, y + 4, 2, 2, COLOR_CUSTOM_TEXT);       // Top dot
    
    // WiFi arcs (custom style)
    display.fillRect(x - 5, y + 7, 2, 1, COLOR_CUSTOM_TEXT);   // Bottom arc
    display.fillRect(x - 3, y + 5, 2, 1, COLOR_CUSTOM_TEXT);   // Middle arc
    display.fillRect(x - 1, y + 3, 2, 1, COLOR_CUSTOM_TEXT);   // Top arc
}

void enterSleepMode() {
    if (!displaySleep) {
        Serial.println("Entering sleep mode...");
        displaySleep = true;
        display.setBrightness(0);  // Turn off backlight
        display.fillScreen(COLOR_BACKGROUND);  // Clear screen
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
        forceRedraw = true;
        
        // Redraw the interface completely
        drawCustomInterface();
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
    
    // Check for touch (if available)
    // Note: T-Watch S3 has touch capability, but we need to implement it
    
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

void drawCustomInterface()
{
    // Get current time
    if (!getLocalTime(&timeinfo)) {
        display.fillScreen(COLOR_BACKGROUND);
        display.setTextColor(COLOR_CUSTOM_WARNING);
        display.setTextSize(2);
        display.setCursor(20, 100);
        display.println("NO SIGNAL");
        return;
    }
    
    // Only redraw everything on first run or when date changes
    static int lastDay = -1;
    static int lastHour = -1;
    static int lastMinute = -1;
    
    if (forceRedraw || lastDay != timeinfo.tm_mday || lastHour != timeinfo.tm_hour || lastMinute != timeinfo.tm_min) {
        // Clear screen and draw background
        display.fillScreen(COLOR_BACKGROUND);
        
        // TODO: Load and display toywatch-darkgrey-alpha.png background
        // For now, we'll use a solid background
        
        // 1. DAY_OF_WEEK - Centralizado horizontalmente
        char dayStr[20];
        const char* days[] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
        sprintf(dayStr, "%s", days[timeinfo.tm_wday]);
        display.setTextColor(COLOR_CUSTOM_TEXT);  // Vermelho puro
        display.setTextSize(3);  // Large text to simulate NexaRustSlab
        display.setCursor(120 - (strlen(dayStr) * 18), 30);  // Centralizado
        display.println(dayStr);
        
        // 2. DATE - Centralizado horizontalmente
        char dateStr[20];
        const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
        sprintf(dateStr, "%02d %s", timeinfo.tm_mday, months[timeinfo.tm_mon]);
        display.setTextSize(3);  // Large text to simulate NexaRustSlab
        display.setCursor(120 - (strlen(dateStr) * 18), 58);  // Centralizado
        display.println(dateStr);
        
        // 3. TIME - Centralizado
        char timeStr[20];
        formatTimeString(timeStr, timeinfo.tm_hour, timeinfo.tm_min);
        display.setTextSize(5);  // Very large text to simulate NexaRustSlab
        display.setCursor(120 - (strlen(timeStr) * 30), 120);  // Centralizado
        display.println(timeStr);
        
        // 4. TEMPERATURE - Alinhado à esquerda
        // TODO: Implementar leitura de temperatura do sensor
        char tempStr[10];
        sprintf(tempStr, "21°C");  // Placeholder
        display.setTextSize(3);  // Large text to simulate NexaRustSlab
        display.setCursor(45, 195);
        display.println(tempStr);
        
        // 5. WIFI_STATUS_ICON - Centralizado horizontalmente
        if (wifiConnected) {
            // Draw WiFi icon
            drawCustomWiFiIcon(120, 195);
        } else {
            // Draw disconnected icon (X)
            display.setTextSize(3);
            display.setCursor(120 - 18, 195);
            display.println("X");
        }
        
        // 6. BATTERY_PERCENTAGE - Alinhado à direita
        char batteryStr[10];
        sprintf(batteryStr, "%d%%", batteryPercent);
        display.setTextSize(3);  // Large text to simulate NexaRustSlab
        display.setCursor(195 - (strlen(batteryStr) * 18), 195);
        display.println(batteryStr);
        
        lastDay = timeinfo.tm_mday;
        lastHour = timeinfo.tm_hour;
        lastMinute = timeinfo.tm_min;
        
        // Reset force redraw flag
        forceRedraw = false;
    }
    
    // Update time when minutes change (no more seconds update)
    static int lastMinuteUpdate = -1;
    if (lastMinuteUpdate != timeinfo.tm_min) {
        // Clear and redraw time area
        display.fillRect(120 - 90, 120, 180, 60, COLOR_BACKGROUND);
        
        // Redraw time with new minutes
        char timeStr[20];
        formatTimeString(timeStr, timeinfo.tm_hour, timeinfo.tm_min);
        display.setTextColor(COLOR_CUSTOM_TEXT);
        display.setTextSize(5);  // Very large text to simulate NexaRustSlab
        display.setCursor(120 - (strlen(timeStr) * 30), 120);
        display.println(timeStr);
        
        lastMinuteUpdate = timeinfo.tm_min;
    }
}

void loop()
{
    // Manage sleep mode
    manageSleepMode();
    
    // Only update display if not in sleep mode
    if (!displaySleep) {
        // Read battery information every 5 seconds
        static unsigned long lastBatteryRead = 0;
        if (millis() - lastBatteryRead > 5000) {
            readBatteryInfo();
            optimizeBatteryUsage();  // Apply battery optimizations
            lastBatteryRead = millis();
        }
        
        drawCustomInterface();  // Use the new custom interface
        delay(1000);
    } else {
        // In sleep mode, just check for activity
        delay(100);  // Shorter delay for faster response
    }
} 