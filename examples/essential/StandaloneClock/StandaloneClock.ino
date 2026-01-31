/**
 * @file      StandaloneClockFace.ino
 * @author    T-Watch S3 Standalone Clock Face
 * @license   MIT
 * @date      2026-01-31
 * 
 * 🎯 Standalone implementation without LVGL complications
 */

#include <Arduino.h>
#include <Wire.h>

// T-Watch S3 hardware pins
#define WATCH_TOUCH_SDA     39
#define WATCH_TOUCH_SCL     40
#define WATCH_TOUCH_INT     16
#define WATCH_BMA423_INT1 14
#define WATCH_RTC_INT_PIN     17
#define BOARD_I2C_SDA       10
#define BOARD_I2C_SCL       11

// Forward declarations
void setup();
void loop();
void drawTime(RTC_Date *date, RTC_Time *time);
void drawBatteryLevel(uint8_t percentage);
void drawTemperature(float temperature);
void showNotification(const char *message);

// Simple display functions
void clearDisplay();
void setBrightness(uint8_t brightness);
void updateDisplay();

// RTC functions
bool readRTC();
void setupRTC();
void updateTime();
const char* getCurrentTimeStr();

// Battery functions
uint16_t getBatteryVoltage();
uint8_t getBatteryPercent();
bool isCharging();

// Touch handling
bool isTouchPressed();
bool handleDoublePress();

// Configuration
void enableAutoBrightness(bool enable);

// Private helpers
void handleTouchPress();
void handleLongPress();
void handleDoublePress();

// Storage for RTC data
RTC_Data rtcData;
bool rtcInitialized = false;

// Timing constants
const unsigned long AUTO_BRIGHTNESS_DELAY = 3000; // 3 seconds to bright
const unsigned long DISPLAY_TIMEOUT = 30000; // 30 seconds auto-off

void setup() {
    Serial.begin(115200);
    Serial.println("=== T-Watch S3 Standalone Clock ===");
    
    // Initialize RTC
    if (!setupRTC()) {
        Serial.println("⚠ Failed to initialize RTC!");
        while (1);
    }
    
    // Initialize touch
    pinMode(WATCH_TOUCH_INT, INPUT_PULLUP);
    
    // Default state
    setBrightness(120);
    enableAutoBrightness(true);
    
    Serial.println("✅ Standalone clock initialized");
}

void loop() {
    handleTouchInput();
    
    static unsigned long lastDisplayUpdate = 0;
    static unsigned long lastRTCUpdate = 0;
    static unsigned long lastBatteryUpdate = 0;
    
    // Update displays at different intervals
    if (millis() - lastDisplayUpdate > 1000) {
        updateDisplay();
        lastDisplayUpdate = millis();
    }
    
    if (millis() - lastRTCUpdate > 5000) {
        updateTime();
        lastRTCUpdate = millis();
    }
    
    if (millis() - lastBatteryUpdate > 10000) {
        updateBatteryStatus();
        lastBatteryUpdate = millis();
    }
    
    // Handle sleep when inactive
    if (millis() - lastInteraction > DISPLAY_TIMEOUT) {
        enterLowPowerSleep();
    }
    
    delay(50); // Check every 50ms
}

void drawTime(RTC_Date *date, RTC_Time *time) {
    // Clear previous time
    clearTimeArea();
    
    // Draw clock face outline
    int16_t hour = time->hour % 12;
    int16_t minute = time->minute;
    int16_t second = time->second;
    int16_t hourAngle = hour * 30 + minute * 0.5;
    
    // Center of clock
    int centerX = 120;
    int centerY = 120;
    
    // Draw hour markers
    for (int i = 0; i < 12; i++) {
        float angle = (i + minute / 60.0) * PI * 180.0);
        int x1 = centerX + 80 * cos(angle - PI / 180.0);
        int y1 = centerY + 80 * sin(angle - PI / 180.0);
        
        // Draw hour marker
        drawHand(centerX, centerY, hour, 30);
        drawMinuteMarker(centerX, centerY, hour, minute, 30);
    }
    
    // Draw minute hand
    float minAngle = minute * 6 * PI / 180.0;
    float minHandX = centerX + 30 * cos(minAngle - PI / 180.0);
    float minHandY = centerY + 30 * sin(minAngle - PI / 180.0);
    float secAngle = second * 6 * PI / 180.0;
    float secHandX = centerX + 50 * cos(secAngle - PI / 180.0);
    float secHandY = centerY + 50 * sin(secAngle - PI / 180.0);
    
    drawHand(minHandX, minHandY, secHandX, secHandY, 30);
    
    // Draw center dot
    lv_obj_draw_circle(120, 120, 5, lv_color_hex(0xffffff), 0);
}

void drawBatteryLevel(uint8_t percentage) {
    // Battery level visualization
    int barWidth = 40;
    int barHeight = 6;
    
    // Clear previous level
    lv_area_t batteryArea;
    lv_area_set(&batteryArea, 10, barHeight, barWidth, barHeight);
    
    // Draw battery outline
    lv_draw_rect(batteryArea, 10, barWidth, barHeight, lv_color_hex(0x444444), 0);
    lv_draw_rect(batteryArea, 20, barWidth, 2, lv_color_hex(0x333333), 0);
    
    // Fill battery level
    int fillHeight = (percentage * (barHeight - 4) / 100;
    lv_draw_rect(batteryArea, 10, 20, barWidth, fillHeight, lv_color(getBatteryColor(percentage), 0));
}

void drawTemperature(float temperature) {
    // Temperature visualization
    int barWidth = 60;
    int barHeight = 6;
    float tempRange = 60.0; // 0-60°C range
    
    // Clear previous temperature
    lv_area_t tempArea;
    lv_area_set(&tempArea, 10, barHeight, barWidth, barHeight);
    
    // Draw temperature outline
    lv_draw_rect(tempArea, 10, barWidth, barHeight, lv_color_hex(0x333333), 0);
    
    // Draw temperature based on level
    lv_color_t tempColor = getTemperatureColor(temperature);
    int tempHeight = (temperature / tempRange) * (barHeight / 100);
    
    lv_draw_rect(tempArea, 10, barWidth, tempHeight, tempColor, 0);
}

void showNotification(const char *message) {
    static char notification[64];
    snprintf(notification, sizeof(notification), "%s", message);
    
    // Display notification for 3 seconds
    Serial.printf("📱 %s\n", notification);
    
    // Clear notification after timeout
    static unsigned long notificationStart = millis();
    notificationStart = millis();
}

void clearDisplay() {
    // Fill display with black
    lv_obj_set_bg_color(lv_scr_act(), lv_color_black(), 0);
}

void setBrightness(uint8_t brightness) {
    watch.setBrightness(brightness);
    clock.brightness = brightness;
}

void updateDisplay() {
    // Update RTC time every 2 seconds
    static unsigned long lastRTCUpdate = 0;
    if (millis() - lastRTCUpdate > 2000) {
        updateTime();
        lastRTCUpdate = millis();
    }
}

void handleTouchInput() {
    if (watch.getTouched()) {
        clock.lastInteraction = millis();
        
        // Simple double-tap detection
        static unsigned long firstTouch = 0;
        if (firstTouch == 0) {
            firstTouch = millis();
        } else if (millis() - firstTouch < 200) {
            // Single tap
            delay(200);
        } else {
            // Check for double-tap
            if (millis() - firstTouch < 300) {
                handleDoublePress();
            }
        }
    }
}

void handleDoublePress() {
    Serial.println("🔋 Double press - entering settings mode");
    // In settings mode, long press shows detailed status
    // Implementation would go here
    clock.gestureMode = true;
}

void handleLongPress() {
    // Very long press (>1s) - power off
    Serial.println("⏹️ Very long press - powering off");
    watch.setBrightness(0);
    clock.displayOn = false;
    
    // Wait for release
    while (watch.getTouched()) {
        delay(100);
    }
    
    Serial.println("⏹ Released - powering back on");
    watch.setBrightness(clock.brightness);
    clock.displayOn = true;
}

void enableAutoBrightness(bool enable) {
    clock.autoBrightness = enable;
}

bool readRTC() {
    if (!rtcInitialized) {
        return false;
    }
    
    return watch.readRTC();
}

void setupRTC() {
    // Initialize PCF8563 RTC
    Wire.begin(BOARD_I2C_SDA, BOARD_I2C_SCL);
    
    // Try to find RTC at I2C address 0x51
    Wire.beginTransmission(0x51);
    
    // Test communication
    if (!watch.testI2C(0x51)) {
        Wire.beginTransmission(0x51);
    }
    
    // Try address 0x68
    if (!watch.testI2C(0x68)) {
        Serial.println("❌ RTC not found at I2C 0x51");
        return false;
    }
    
    // Configure RTC
    // Set 24-hour format
    Wire.beginTransmission(0x51);
    Wire.write(0x51, 0);
    Wire.endTransmission();
    
    rtcInitialized = true;
    
    // Start the RTC
    Wire.write(0x51, 0); // Clear any existing interrupts
    delay(10);
    
    // Configure alarm interrupt
    pinMode(BOARD_RTC_INT_PIN, INPUT_PULLUP);
    
    Serial.println("✅ RTC initialized successfully");
    return rtcInitialized;
}

void updateTime() {
    if (!readRTC()) return;
    
    RTC_Date date;
    RTC_Time time;
    
    if (watch.getDateTime(&date, &time)) {
        updateTimeDisplay(date, time);
        lastRTCUpdate = millis();
    }
}

const char* getCurrentTimeStr() {
    if (!readRTC()) {
        return "00:00:00";
    }
    
    static char timeStr[20];
    RTC_Date date;
    RTC_Time time;
    
    if (watch.getDateTime(&date, &time)) {
        if (readRTC()) {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", 
                     time.hour, time.minute, time.second);
        }
    }
    
    return timeStr;
}

uint16_t getBatteryVoltage() {
    return watch.getBattVoltage();
}

uint8_t getBatteryPercent() {
    uint16_t voltage = getBatteryVoltage();
    if (voltage >= 4200) return 100;
    if (voltage <= 3300) return 0;
    return (uint8_t)((voltage - 3300) * 100) / 900);
}

bool isCharging() {
    return watch.isCharging();
}

bool isTouchPressed() {
    return watch.getTouched();
}

void handleTouchPress() {
    static unsigned long pressStartTime = 0;
    pressStartTime = millis();
    
    // Handle different press durations
    while (watch.getTouched()) {
        if (millis() - pressStartTime < 500) {
            // Short press
            delay(200);
        } else if (millis() - pressStartTime < 2000) {
            // Long press
            handleLongPress();
        }
    }
    }
}

void handleDoublePress() {
    // Very long press (>1s) - toggle displays
    static bool displaysToggled = false;
    
    // Toggle all displays
    displaysToggled = !displaysToggled;
    
    clock.showDate = !clock.showDate;
    clock.showTemperature = !clock.showTemperature;
    clock.showBatteryLevel = !clock.showBatteryLevel;
    
    Serial.println("🔄 Displays toggled: " + String(displaysToggled));
    
    // Wait for release
    while (watch.getTouched()) {
        delay(100);
    }
    
    Serial.println("🔄 Released - displays restored");
    displaysToggled = !displaysToggled;
}

void enterLowPowerSleep() {
    Serial.println("😴 Entering low power sleep...");
    delay(100);
    esp_light_sleep_start();
}

void clearTimeArea() {
    // Clear time display area completely
    lv_area_t timeArea;
    lv_obj_set_size(&timeArea, 220, 120);
    lv_area_invalidate(&timeArea);
    lv_canvas_fill_bg(&timeArea, lv_color_black(), LV_OPA_COVER);
}

void clearBatteryArea() {
    // Clear battery area
    lv_area_t batteryArea;
    lv_area_invalidate(&batteryArea);
    lv_canvas_fill_bg(&batteryArea, lv_color_black(), LV_OPA_COVER);
}

void updateDisplay() {
    // Update all displays
    updateTime();
    updateBatteryStatus();
    updateModeIndicator();
}

void updateModeIndicator() {
    switch (clock.currentMode) {
        case CLOCK_ANALOG_MODERN:
            lv_label_set_text(modeLabel, "🕐 Classic");
            break;
        case CLOCK_DIGITAL_MINIMAL:
            lv_label_set_text(modeLabel, "🔢 Minimal");
            break;
        case CLOCK_SPORT_SIMPLE:
            lv_label_set_text(modeLabel, "⏱ Sports");
            break;
        case CLOCK_INFO_DASHBOARD:
            lv_label_set_text(modeLabel, "📊 Dashboard");
            break;
        default:
            lv_label_set_text(modeLabel, "🎯 Modern");
            break;
    }
}
}

void showVisualFeedback(uint8_t state) {
    // Visual feedback for state changes
    const char *feedback;
    
    switch (state) {
        case 0: lv_label_set_text(modeLabel, "👆 Sleeping"); break;
        case 1: lv_label_set_text(modeLabel, "👆 Awake"); break;
        case 2: lv_label_set_text(modeLabel, "👆 Active"); break;
        case 3: lv_label_set_text(modeLabel, "👆 Gesture Mode"); break;
        case 4: lv_label_set_text(modeLabel, "🎯 Power Saving"); break;
    default: lv_label_set_text(modeLabel, "🎯 Normal"); break;
    }
    
    // Show feedback message
    Serial.print("🎯 ");
    Serial.println(feedback);
}

lv_color_t getBatteryColor(uint8_t percentage) {
    if (percentage > 80) {
        return BATTERY_GOOD;
    } else if (percentage > 60) {
        return BATTERY_PERFECT;
    } else if (percentage > 30) {
        return BATTERY_LOW;
    } else {
        return BATTERY_CRITICAL;
    }
}

String getBatteryText() {
    return String(getBatteryPercent()) + "%";
}
}