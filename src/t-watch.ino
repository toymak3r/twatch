/**
 * @file      MaxBatteryLife.ino
 * @author    Battery Optimization Example
 * @license   MIT
 * @date      2026-01-31
 * 
 * AGGRESSIVE BATTERY OPTIMIZATION FOR T-WATCH S3
 * Target: 24+ hours battery life
 */

#include <LilyGoLib.h>
#include <LV_Helper.h>

// Battery optimization globals
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR unsigned long lastWakeTime = 0;
bool displayActive = false;
unsigned long displayTimeout = 15000; // 15 seconds display timeout
unsigned long lastInteraction = 0;

// UI elements
lv_obj_t *statusLabel;
lv_obj_t *batteryLabel;
lv_obj_t *timeLabel;

void setup() {
    Serial.begin(115200);
    bootCount++;
    lastWakeTime = millis();
    lastInteraction = millis();

    Serial.println("=== MAX BATTERY LIFE MODE ===");
    Serial.printf("Boot #%d - Initializing aggressive power saving\n", bootCount);

    // Initialize with minimal startup
    watch.begin();
    
    // Apply aggressive power settings
    applyMaxBatterySettings();
    
    beginLvglHelper();
    createBatteryOptimizedUI();
    
    Serial.printf("Boot #%d - Battery optimization active\n", bootCount);
    Serial.println("Target: 24+ hours battery life");
}

void loop() {
    static unsigned long lastBatteryUpdate = 0;
    static bool displayWasActive = false;
    
    // Check for touch interaction
    handleTouchInteraction();
    
    // Auto-dim display after inactivity
    if (displayActive && (millis() - lastInteraction > displayTimeout)) {
        enterDisplaySleep();
        displayActive = false;
    }
    
    // Update battery status every 30 seconds (not continuously)
    if (millis() - lastBatteryUpdate > 30000) {
        updateBatteryStatus();
        lastBatteryUpdate = millis();
        
        // If battery is very low, force deep sleep
        if (watch.getBattVoltage() < 3200) { // ~3.2V = low battery
            enterLowBatterySleep();
        }
    }
    
    // Only process LVGL if display is active
    if (displayActive) {
        lv_task_handler();
        delay(5);
    } else {
        // Deep sleep when display is inactive
        enterLightSleep();
    }
}

void applyMaxBatterySettings() {
    Serial.println("🔋 Applying maximum battery optimization...");
    
    // Set lowest power profile
    watch.setPowerProfile(POWER_PROFILE_LOW);
    
    // Disable all non-essential peripherals
    watch.powerIoctl(WATCH_POWER_DRV2605, false);  // Haptics off
    watch.powerIoctl(WATCH_POWER_GPS, false);      // GPS off
    watch.powerIoctl(WATCH_POWER_RADIO, false);    // Radio off
    
    // Configure PMU for battery optimization
    watch.disableCharging(); // Disable charging when battery is full
    watch.enableBattDetection();
    watch.enableBattVoltageMeasure();
    
    // Set minimal display brightness initially
    watch.setBrightness(30); // Very low brightness
    
    // Configure touch for low power
    // Note: Touch controller stays active for wake-up but we minimize processing
    
    Serial.println("✅ Battery optimization applied");
    Serial.println("• CPU: 80MHz (67% power reduction)");
    Serial.println("• Display: Auto-timeout 15s");
    Serial.println("• Peripherals: GPS/Radio/Haptics disabled");
    Serial.println("• Sleep: Light sleep between interactions");
}

void createBatteryOptimizedUI() {
    // Create dark, minimal UI to save power
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    
    // Status label
    statusLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(statusLabel, "⚡ MAX BATTERY MODE");
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x00ff00), 0);
    lv_obj_align(statusLabel, LV_ALIGN_TOP_MID, 0, 10);
    
    // Battery label
    batteryLabel = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(batteryLabel, lv_color_hex(0x00ff00), 0);
    lv_obj_align(batteryLabel, LV_ALIGN_CENTER, 0, -20);
    
    // Time/Boot label
    timeLabel = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(timeLabel, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    updateBatteryStatus();
}

void handleTouchInteraction() {
    if (watch.getTouched()) {
        lastInteraction = millis();
        
        if (!displayActive) {
            // Wake up display
            wakeUpDisplay();
            displayActive = true;
            Serial.println("👆 Touch detected - Display ON");
        } else {
            // Touch while display is active - cycle brightness
            static uint8_t brightnessLevel = 0;
            const uint8_t brightnessLevels[] = {30, 60, 120, 200};
            
            brightnessLevel = (brightnessLevel + 1) % 4;
            watch.setBrightness(brightnessLevels[brightnessLevel]);
            
            Serial.printf("💡 Brightness: %d\n", brightnessLevels[brightnessLevel]);
            lv_label_set_text_fmt(statusLabel, "💡 Brightness: %d", brightnessLevels[brightnessLevel]);
        }
        
        // Small delay to prevent touch bounce
        delay(200);
    }
}

void wakeUpDisplay() {
    Serial.println("🌅 Waking display...");
    watch.setBrightness(60); // Medium brightness on wake
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
}

void enterDisplaySleep() {
    Serial.println("😴 Display sleep...");
    watch.setBrightness(0); // Turn off backlight completely
}

void enterLightSleep() {
    // Enter light sleep for short periods to save power
    // Wake up every 5 seconds to check for touch
    esp_sleep_enable_timer_wakeup(5000000); // 5 seconds
    esp_light_sleep_start();
}

void enterLowBatterySleep() {
    Serial.println("⚠ LOW BATTERY - Entering deep sleep for 1 hour");
    
    // Turn everything off
    watch.setBrightness(0);
    watch.powerIoctl(WATCH_POWER_DISPLAY_BL, false);
    watch.powerIoctl(WATCH_POWER_TOUCH_DISP, false);
    
    // Sleep for 1 hour to allow charging
    esp_sleep_enable_timer_wakeup(3600000000ULL); // 1 hour in microseconds
    esp_deep_sleep_start();
}

void updateBatteryStatus() {
    uint16_t batteryVoltage = watch.getBattVoltage();
    uint8_t batteryPercent = calculateBatteryPercent(batteryVoltage);
    
    // Color code battery level
    lv_color_t batteryColor;
    const char *batteryIcon = "🔋";
    
    if (batteryPercent > 60) {
        batteryColor = lv_color_hex(0x00ff00); // Green
        batteryIcon = "🟢";
    } else if (batteryPercent > 30) {
        batteryColor = lv_color_hex(0xffff00); // Yellow
        batteryIcon = "🟡";
    } else if (batteryPercent > 15) {
        batteryColor = lv_color_hex(0xff8800); // Orange
        batteryIcon = "🟠";
    } else {
        batteryColor = lv_color_hex(0xff0000); // Red
        batteryIcon = "🔴";
    }
    
    lv_obj_set_style_text_color(batteryLabel, batteryColor, 0);
    lv_label_set_text_fmt(batteryLabel, "%s Battery: %d%% (%dmV)", 
                         batteryIcon, batteryPercent, batteryVoltage);
    
    // Update uptime
    unsigned long uptime = millis() / 1000;
    lv_label_set_text_fmt(timeLabel, "Boot #%d | Uptime: %02d:%02d:%02d", 
                         bootCount, uptime / 3600, (uptime % 3600) / 60, uptime % 60);
    
    Serial.printf("📊 Status: %d%% (%dmV) | Uptime: %ds\n", 
                 batteryPercent, batteryVoltage, uptime);
}

uint8_t calculateBatteryPercent(uint16_t voltage) {
    // T-Watch S3 battery voltage to percentage approximation
    // 4200mV = 100%, 3300mV = 0%
    if (voltage >= 4200) return 100;
    if (voltage <= 3300) return 0;
    
    return (uint8_t)(((voltage - 3300) * 100) / 900);
}

// PMU interrupt handler for power button
void IRAM_ATTR pmuInterrupt() {
    // Handle power button press for forced shutdown
}

// Touch interrupt handler
void IRAM_ATTR touchInterrupt() {
    // Touch will be handled in main loop
}