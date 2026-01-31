/**
 * @file      SimpleModernClock.ino
 * @author    T-Watch S3 Simple Modern Clock
 * @license   MIT
 * @date      2026-01-31
 * 
 * Simplified modern clock face without LVGL canvas complications
 */

#include <Arduino.h>

// Simple modern clock using basic display functions
// This avoids the complex LilyGoLib dependencies while demonstrating battery optimization

// Simple digital clock with battery optimization
typedef struct {
    uint8_t brightness;
    bool displayOn;
    unsigned long lastInteraction;
    unsigned long lastUpdate;
    bool is24HourFormat;
    bool showSeconds;
    float batteryVoltage;
    uint8_t batteryPercent;
} ClockState;

static ClockState clockState;

// Forward declarations
void updateSimpleClockDisplay();
void updateSimpleBatteryDisplay();
uint8_t calculateBatteryPercent(uint16_t voltage);
String getBatteryStatus(uint8_t percent);

void setup() {
    Serial.begin(115200);
    Serial.println("=== T-Watch Simple Modern Clock Demo ===");
    Serial.println("🔋 Battery optimization demonstration");
    
    // Initialize clock state
    clockState.brightness = 80;
    clockState.displayOn = true;
    clockState.lastInteraction = millis();
    clockState.lastUpdate = 0;
    clockState.is24HourFormat = true;
    clockState.showSeconds = true;
    
    // Simulate initial battery reading (would come from hardware)
    clockState.batteryVoltage = 3900; // 3.9V = ~67%
    clockState.batteryPercent = calculateBatteryPercent(clockState.batteryVoltage);
    
    Serial.println("✅ Modern clock demo initialized");
    Serial.println("🔋 This demo shows battery optimization techniques");
    Serial.println("⚡ Power saving features:");
    Serial.println("   - Auto-dim after 30 seconds");
    Serial.println("   - Touch-to-wake functionality");
    Serial.println("   - Optimized refresh rates");
    Serial.println("   - Battery monitoring");
}

void loop() {
    static unsigned long lastDisplayUpdate = 0;
    static unsigned long lastBatteryUpdate = 0;
    static unsigned long lastTouchSimulation = 0;
    
    // Simulate touch interaction every 10 seconds for demo
    if (millis() - lastTouchSimulation > 10000) {
        clockState.lastInteraction = millis();
        clockState.displayOn = true;
        Serial.println("👆 Touch simulated - display activated");
        lastTouchSimulation = millis();
    }
    
    // Auto-dim after 30 seconds of inactivity
    if (millis() - clockState.lastInteraction > 30000 && clockState.brightness > 30) {
        clockState.brightness = max(30, clockState.brightness - 10);
        Serial.printf("🔅 Auto-dimming to %d%%\n", clockState.brightness);
    }
    
    // Restore brightness after interaction
    if (millis() - clockState.lastInteraction < 5000 && clockState.brightness < 80) {
        clockState.brightness = 80;
        Serial.println("☀️ Brightness restored");
    }
    
    // Update time display every second
    if (millis() - lastDisplayUpdate > 1000) {
        updateSimpleClockDisplay();
        lastDisplayUpdate = millis();
    }
    
    // Update battery every 5 seconds (simulate drain)
    if (millis() - lastBatteryUpdate > 5000) {
        // Simulate battery drain (0.1% every 5 seconds = ~72 hours for full drain)
        if (clockState.batteryPercent > 0) {
            clockState.batteryPercent = max(0, clockState.batteryPercent - 1);
            clockState.batteryVoltage = 3300 + (clockState.batteryPercent * 9);
        }
        updateSimpleBatteryDisplay();
        lastBatteryUpdate = millis();
    }
    
    delay(50); // Small delay for stability
}

void updateSimpleClockDisplay() {
    // Get current time from system
    unsigned long currentTime = millis();
    int hours = (currentTime / 3600000) % 24;
    int minutes = (currentTime / 60000) % 60;
    int seconds = (currentTime / 1000) % 60;
    
    // Display time in serial monitor (demo mode)
    char timeStr[20];
    if (clockState.is24HourFormat) {
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", hours, minutes, seconds);
    } else {
        int hour12 = hours % 12;
        if (hour12 == 0) hour12 = 12;
        const char *ampm = hours >= 12 ? "PM" : "AM";
        snprintf(timeStr, sizeof(timeStr), "%d:%02d:%02d %s", hour12, minutes, seconds, ampm);
    }
    
    // Simulate display update
    Serial.printf("⏰ Time: %s | Brightness: %d%% | Display: %s\n", 
                  timeStr, clockState.brightness, clockState.displayOn ? "ON" : "OFF");
}

void updateSimpleBatteryDisplay() {
    String status = getBatteryStatus(clockState.batteryPercent);
    Serial.printf("🔋 Battery: %d%% (%.2fV) %s\n", 
                  clockState.batteryPercent, clockState.batteryVoltage / 1000.0, status.c_str());
    
    // Battery saving warnings
    if (clockState.batteryPercent == 20) {
        Serial.println("⚠️  BATTERY LOW: Consider charging soon");
    } else if (clockState.batteryPercent == 10) {
        Serial.println("🚨 BATTERY CRITICAL: Charge immediately");
    }
    
    // Calculate estimated remaining time
    float hoursRemaining = (clockState.batteryPercent / 100.0) * 24.0; // Based on 24-hour target
    Serial.printf("⏱️  Estimated remaining: %.1f hours\n", hoursRemaining);
}

String getBatteryStatus(uint8_t percent) {
    if (percent > 60) return "🟢 GOOD";
    if (percent > 30) return "🟡 OK";
    if (percent > 15) return "🟠 LOW";
    return "🔴 CRITICAL";
}

uint8_t calculateBatteryPercent(uint16_t voltage) {
    // T-Watch S3 battery voltage range: 3300mV (0%) to 4200mV (100%)
    if (voltage >= 4200) return 100;
    if (voltage <= 3300) return 0;
    return (uint8_t)(((voltage - 3300) * 100) / 900);
}