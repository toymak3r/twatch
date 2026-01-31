/**
 * @file      SensorBasics.ino
 * @author    T-Watch S3 Sensor Basics
 * @license   MIT
 * @date      2026-01-31
 * 
 * ESSENTIAL EXAMPLE - Accelerometer and RTC
 * Learn: BMA423 accelerometer, step counting, PCF8563 RTC
 */

#include <LilyGoLib.h>
#include <LV_Helper.h>

// Sensor globals
lv_obj_t *sensorLabel;
lv_obj_t *stepLabel;
lv_obj_t *timeLabel;
lv_obj_t *tempLabel;
uint32_t lastStepCount = 0;
unsigned long lastUpdate = 0;

void setup() {
    Serial.begin(115200);
    Serial.println("=== T-Watch Sensor Basics ===");

    watch.begin();
    beginLvglHelper();
    
    createSensorUI();
    
    // Attach sensor interrupt for step detection
    watch.attachBMA(onStepDetected);
    
    // Clear any existing step count for fresh start
    watch.readBMA();
    
    Serial.println("✅ Sensors initialized");
    Serial.println("🚶‍♂️ Move watch to count steps");
}

void loop() {
    updateSensorData();
    
    lv_task_handler();
    delay(100); // 10Hz update rate
}

void createSensorUI() {
    // Dark theme
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    
    // Title
    sensorLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(sensorLabel, "📊 T-Watch Sensors");
    lv_obj_set_style_text_color(sensorLabel, lv_color_hex(0x00ff00), 0);
    lv_obj_align(sensorLabel, LV_ALIGN_TOP_MID, 0, 10);
    
    // Step counter
    stepLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(stepLabel, "Steps: 0");
    lv_obj_set_style_text_color(stepLabel, lv_color_hex(0x00ffff), 0);
    lv_obj_align(stepLabel, LV_ALIGN_CENTER, 0, -30);
    
    // Temperature
    tempLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(tempLabel, "Temperature: --.-°C");
    lv_obj_set_style_text_color(tempLabel, lv_color_hex(0xffff00), 0);
    lv_obj_align(tempLabel, LV_ALIGN_CENTER, 0, 0);
    
    // RTC time
    timeLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(timeLabel, "Time: --:--:--");
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(timeLabel, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    watch.setBrightness(100);
}

void updateSensorData() {
    // Update every second
    if (millis() - lastUpdate < 1000) return;
    lastUpdate = millis();
    
    // Read accelerometer data
    uint16_t irqStatus = watch.readBMA();
    bool isStepDetected = (irqStatus & 0x80) != 0; // Step detected bit
    
    // Get step count
    uint32_t stepCount = watch.getStepCounter();
    
    // Display step count
    lv_label_set_text_fmt(stepLabel, "🚶 Steps: %lu", stepCount);
    
    // Update temperature from accelerometer
    float accelTemp = watch.readAccelTemp();
    lv_label_set_text_fmt(tempLabel, "🌡️ Temp: %.1f°C", accelTemp);
    
    // Update RTC time
    updateRTC();
    
    // Show step detection indicator
    if (isStepDetected && stepCount > lastStepCount) {
        lv_obj_set_style_text_color(stepLabel, lv_color_hex(0xff00ff), 0); // Purple flash
        lastStepCount = stepCount;
        Serial.printf("🚶‍♂️ Step detected! Total: %lu\n", stepCount);
    } else {
        lv_obj_set_style_text_color(stepLabel, lv_color_hex(0x00ffff), 0);
    }
}

void updateRTC() {
    // Get current time from RTC
    RTC_Date now;
    RTC_Time tim;
    
    if (watch.readRTC() && watch.getDateTime(&now, &tim)) {
        lv_label_set_text_fmt(timeLabel, "🕐 %04d/%02d/%02d %02d:%02d:%02d", 
                             now.year, now.month, now.day, 
                             tim.hour, tim.minute, tim.second);
    } else {
        lv_label_set_text(timeLabel, "🕐 RTC not available");
    }
}

// Step detection interrupt handler
void IRAM_ATTR onStepDetected() {
    // Step is processed in main loop via readBMA()
}

// Utility to show sensor calibration
void calibrateSensors() {
    Serial.println("🔧 Calibrating sensors...");
    
    // Reset step counter
    watch.resetStepCounter();
    
    // Set accelerometer sensitivity (0 = highest, 7 = lowest)
    // watch.setAccelSensitivity(2);
    
    Serial.println("✅ Sensors calibrated");
}