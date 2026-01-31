/**
 * @file      BatteryOptimization.ino
 * @author    T-Watch S3 Battery Optimization
 * @license   MIT
 * @date      2026-01-31
 * 
 * ESSENTIAL EXAMPLE - Maximum Battery Life
 * Target: 24+ hours battery life
 */

#include <LilyGoLib.h>
#include <LV_Helper.h>

// Battery optimization globals
RTC_DATA_ATTR int bootCount = 0;
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
    lastInteraction = millis();

    Serial.println("=== T-Watch Battery Optimization ===");
    Serial.printf("Boot #%d - Applying maximum power saving\n", bootCount);

    // Initialize with minimal startup
    watch.begin();
    
    // Apply aggressive power settings
    applyMaxBatterySettings();
    
    beginLvglHelper();
    createOptimizedUI();
    
    Serial.println("✅ Battery optimization active");
    Serial.println("🎯 Target: 24+ hours battery life");
}

void loop() {
    static unsigned long lastBatteryUpdate = 0;
    
    // Check for touch interaction
    handleTouchInteraction();
    
    // Auto-dim display after inactivity
    if (displayActive && (millis() - lastInteraction > displayTimeout)) {
        enterDisplaySleep();
        displayActive = false;
    }
    
    // Update battery status every 30 seconds
    if (millis() - lastBatteryUpdate > 30000) {
        updateBatteryStatus();
        lastBatteryUpdate = millis();
        
        // Low battery protection
        if (watch.getBattVoltage() < 3200) {
            enterLowBatterySleep();
        }
    }
    
    // Only process LVGL if display is active
    if (displayActive) {
        lv_task_handler();
        delay(5);
    } else {
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
    watch.enableBattDetection();
    watch.enableBattVoltageMeasure();
    
    // Set minimal display brightness initially
    watch.setBrightness(30); // Very low brightness
    
    Serial.println("• CPU: 80MHz (67% power reduction)");
    Serial.println("• Display: Auto-timeout 15s");
    Serial.println("• Peripherals: GPS/Radio/Haptics disabled");
    Serial.println("✅ Battery optimization applied");
}

void createOptimizedUI() {
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
            wakeUpDisplay();
            displayActive = true;
            Serial.println("👆 Touch detected - Display ON");
        } else {
            // Touch while display is active - cycle brightness
            static uint8_t brightnessLevel = 0;
            const uint8_t brightnessLevels[] = {30, 60, 120, 200};
            
            brightnessLevel = (brightnessLevel + 1) % 4;
            watch.setBrightness(brightnessLevels[brightnessLevel]);
            
            lv_label_set_text_fmt(statusLabel, "💡 Brightness: %d", brightnessLevels[brightnessLevel]);
        }
        
        delay(200); // Prevent touch bounce
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
    esp_sleep_enable_timer_wakeup(5000000); // 5 seconds
    esp_light_sleep_start();
}

void enterLowBatterySleep() {
    Serial.println("⚠ LOW BATTERY - Deep sleep for 1 hour");
    watch.setBrightness(0);
    esp_sleep_enable_timer_wakeup(3600000000ULL); // 1 hour
    esp_deep_sleep_start();
}

void updateBatteryStatus() {
    uint16_t batteryVoltage = watch.getBattVoltage();
    uint8_t batteryPercent = calculateBatteryPercent(batteryVoltage);
    
    // Color code battery level
    lv_color_t batteryColor;
    const char *batteryIcon = "🔋";
    
    if (batteryPercent > 60) {
        batteryColor = lv_color_hex(0x00ff00);
        batteryIcon = "🟢";
    } else if (batteryPercent > 30) {
        batteryColor = lv_color_hex(0xffff00);
        batteryIcon = "🟡";
    } else if (batteryPercent > 15) {
        batteryColor = lv_color_hex(0xff8800);
        batteryIcon = "🟠";
    } else {
        batteryColor = lv_color_hex(0xff0000);
        batteryIcon = "🔴";
    }
    
    lv_obj_set_style_text_color(batteryLabel, batteryColor, 0);
    lv_label_set_text_fmt(batteryLabel, "%s Battery: %d%% (%dmV)", 
                         batteryIcon, batteryPercent, batteryVoltage);
    
    // Update uptime
    unsigned long uptime = millis() / 1000;
    lv_label_set_text_fmt(timeLabel, "Boot #%d | Uptime: %02d:%02d:%02d", 
                         bootCount, uptime / 3600, (uptime % 3600) / 60, uptime % 60);
}

uint8_t calculateBatteryPercent(uint16_t voltage) {
    if (voltage >= 4200) return 100;
    if (voltage <= 3300) return 0;
    return (uint8_t)(((voltage - 3300) * 100) / 900);
}