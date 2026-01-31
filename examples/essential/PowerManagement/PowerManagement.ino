/**
 * @file      PowerManagement.ino
 * @author    T-Watch S3 Power Management
 * @license   MIT
 * @date      2026-01-31
 * 
 * ESSENTIAL EXAMPLE - PMU and Sleep Modes
 * Learn: AXP2101 power management, sleep configurations, wake-up sources
 */

#include <LilyGoLib.h>
#include <LV_Helper.h>

// Power management globals
lv_obj_t *powerLabel;
lv_obj_t *pmuLabel;
lv_obj_t *wakeLabel;
bool pmuFlag = false;

void setup() {
    Serial.begin(115200);
    Serial.println("=== T-Watch Power Management ===");

    watch.begin();
    beginLvglHelper();
    
    createPowerUI();
    
    // Configure PMU interrupts
    setupPMUInterrupts();
    
    Serial.println("✅ Power management initialized");
    Serial.println("⚡ Press power button to test wake-up");
}

void loop() {
    static unsigned long lastUpdate = 0;
    
    // Handle PMU interrupts
    if (pmuFlag) {
        handlePMUInterrupt();
        pmuFlag = false;
    }
    
    // Update power status every 2 seconds
    if (millis() - lastUpdate > 2000) {
        updatePowerStatus();
        lastUpdate = millis();
    }
    
    lv_task_handler();
    delay(50);
}

void createPowerUI() {
    // Dark theme
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    
    // Title
    powerLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(powerLabel, "⚡ Power Management");
    lv_obj_set_style_text_color(powerLabel, lv_color_hex(0x00ff00), 0);
    lv_obj_align(powerLabel, LV_ALIGN_TOP_MID, 0, 10);
    
    // PMU status
    pmuLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(pmuLabel, "Battery: --.-V | Status: ---");
    lv_obj_set_style_text_color(pmuLabel, lv_color_hex(0x00ffff), 0);
    lv_obj_align(pmuLabel, LV_ALIGN_CENTER, 0, -20);
    
    // Wake-up info
    wakeLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(wakeLabel, "Wake source: None");
    lv_obj_set_style_text_color(wakeLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(wakeLabel, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    watch.setBrightness(100);
}

void setupPMUInterrupts() {
    // Attach PMU interrupt handler
    watch.attachPMU(onPMUInterrupt);
    
    // Enable specific PMU interrupts
    watch.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
    watch.clearPMU();
    
    // Enable power button, battery, and charging interrupts
    watch.enableIRQ(
        XPOWERS_AXP2101_PKEY_SHORT_IRQ |    // Power button short press
        XPOWERS_AXP2101_PKEY_LONG_IRQ |     // Power button long press
        XPOWERS_AXP2101_BAT_INSERT_IRQ |    // Battery inserted
        XPOWERS_AXP2101_BAT_REMOVE_IRQ |    // Battery removed
        XPOWERS_AXP2101_VBUS_INSERT_IRQ |   // USB plugged in
        XPOWERS_AXP2101_VBUS_REMOVE_IRQ |   // USB unplugged
        XPOWERS_AXP2101_BAT_CHG_DONE_IRQ |  // Charging complete
        XPOWERS_AXP2101_BAT_CHG_START_IRQ   // Charging started
    );
    
    Serial.println("🔌 PMU interrupts configured");
}

void updatePowerStatus() {
    uint16_t batteryVoltage = watch.getBattVoltage();
    uint8_t batteryPercent = calculateBatteryPercent(batteryVoltage);
    bool isCharging = watch.isCharging();
    bool isVbusConnected = watch.isVbusIn();
    
    // Update battery status
    lv_label_set_text_fmt(pmuLabel, "🔋 Battery: %d.%02dV | %d%% | %s", 
                         batteryVoltage / 1000, (batteryVoltage % 1000) / 10,
                         batteryPercent,
                         isCharging ? "Charging" : "Not Charging");
    
    // Update battery color based on level
    lv_color_t batteryColor;
    if (batteryPercent > 60) {
        batteryColor = lv_color_hex(0x00ff00); // Green
    } else if (batteryPercent > 30) {
        batteryColor = lv_color_hex(0xffff00); // Yellow
    } else {
        batteryColor = lv_color_hex(0xff0000); // Red
    }
    lv_obj_set_style_text_color(pmuLabel, batteryColor, 0);
    
    // Print serial info
    Serial.printf("📊 Battery: %dmV (%d%%) | Charging: %s | VBUS: %s\n",
                 batteryVoltage, batteryPercent,
                 isCharging ? "Yes" : "No",
                 isVbusConnected ? "Connected" : "Disconnected");
}

void handlePMUInterrupt() {
    uint64_t status = watch.readPMU();
    
    Serial.printf("🔌 PMU Interrupt: 0x%08llX\n", status);
    
    // Check specific interrupt sources
    if (watch.isPekeyShortPressIrq()) {
        Serial.println("👆 Power button short press");
        lv_label_set_text(wakeLabel, "Wake: Power button (short)");
    }
    
    if (watch.isPekeyLongPressIrq()) {
        Serial.println("⏰ Power button long press - Entering deep sleep");
        lv_label_set_text(wakeLabel, "Wake: Power button (long)");
        
        // Enter deep sleep for 10 seconds
        watch.setSleepMode(PMU_BTN_WAKEUP);
        watch.sleep(10);
    }
    
    if (watch.isVbusInsertIrq()) {
        Serial.println("🔌 USB connected");
        lv_label_set_text(wakeLabel, "Wake: USB connected");
    }
    
    if (watch.isVbusRemoveIrq()) {
        Serial.println("🔌 USB disconnected");
        lv_label_set_text(wakeLabel, "Wake: USB disconnected");
    }
    
    if (watch.isBatChagerStartIrq()) {
        Serial.println("⚡ Charging started");
        lv_label_set_text(wakeLabel, "Wake: Charging started");
    }
    
    if (watch.isBatChagerDoneIrq()) {
        Serial.println("✅ Charging complete");
        lv_label_set_text(wakeLabel, "Wake: Charging complete");
    }
    
    // Clear interrupt status
    watch.clearPMU();
}

// PMU interrupt handler
void IRAM_ATTR onPMUInterrupt() {
    pmuFlag = true;
}

uint8_t calculateBatteryPercent(uint16_t voltage) {
    if (voltage >= 4200) return 100;
    if (voltage <= 3300) return 0;
    return (uint8_t)(((voltage - 3300) * 100) / 900);
}

// Advanced power management functions
void enterDeepSleepDemo() {
    Serial.println("😴 Entering deep sleep for 30 seconds...");
    Serial.println("Press power button to wake up");
    
    // Set wake-up source and sleep duration
    watch.setSleepMode(PMU_BTN_WAKEUP);
    watch.sleep(30);
}

void enterLightSleepDemo() {
    Serial.println("😴 Entering light sleep for 10 seconds...");
    Serial.println("Touch screen to wake up");
    
    // Configure touch wake-up and light sleep
    watch.setSleepMode(TOUCH_WAKEUP);
    esp_sleep_enable_timer_wakeup(10000000); // 10 seconds
    esp_light_sleep_start();
}