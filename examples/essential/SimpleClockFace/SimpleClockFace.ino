/**
 * @file      SimpleClockFace.ino
 * @author    T-Watch S3 Simple Improved Clock
 * @license   MIT
 * @date      2026-01-31
 * 
 * SIMPLIFIED CLOCK WITH BETTER UX
 * No LVGL complications, just essential features
 */

#include <LilyGoLib.h>

// Clock configuration
typedef enum {
    CLOCK_SIMPLE_ANALOG = 0,
    CLOCK_SIMPLE_DIGITAL = 1,
    CLOCK_BATTERY_OPTIMIZED = 2
} ClockMode;

// State tracking
typedef struct {
    ClockMode currentMode;
    uint8_t brightness;
    bool displayOn;
    unsigned long lastInteraction;
    unsigned long lastUpdate;
    unsigned long lastButtonPress;
    bool is24HourFormat;
    bool showDate;
    unsigned long lastStepCount;
    float batteryVoltage;
    uint8_t batteryPercent;
} ClockState;

// UI elements
static uint8_t timeX, timeY;
static const int centerX = 120;
static const int centerY = 120;

void setup() {
    Serial.begin(115200);
    Serial.println("=== T-Watch S3 Simple Clock UX ===");
    Serial.println("🎯 Battery optimized, gesture controls");
    
    if (!watch.begin()) {
        Serial.println("❌ Failed to initialize T-Watch");
        return;
    }
    
    initializeClockState();
    createSimpleClockUI();
    setupGestures();
    
    applyClockOptimizations();
    
    Serial.println("✅ Simple clock face initialized");
    Serial.println("👆 Swipe up/down: change faces");
    Serial.println("👆 Double-tap: toggle displays");
    Serial.println("👆 Hold 2s: brightness control");
    Serial.println("👆 Left swipe: gesture mode");
    Serial.println("👆 Right swipe: brightness up");
}

void initializeClockState() {
    clock.currentMode = CLOCK_SIMPLE_ANALOG;
    clock.brightness = 100;
    clock.displayOn = true;
    clock.lastInteraction = millis();
    clock.lastUpdate = 0;
    clock.lastButtonPress = 0;
    clock.is24HourFormat = true;
    clock.showDate = false;
    clock.lastStepCount = 0;
    clock.batteryVoltage = watch.getBattVoltage();
    clock.batteryPercent = calculateBatteryPercent(clock.batteryVoltage);
    
    // Initialize touch coordinates
    watch.getPoint(&timeX, &timeY);
}

void createSimpleClockUI() {
    // Clear screen
    watch.fillScreen(lv_color_black());
    watch.setBrightness(clock.brightness);
    
    // Create main time display
    lv_obj_t *timeLabel = lv_label_create(lv_scr_act());
    lv_label_set_text_font(timeLabel, "00:00:00");
    lv_label_set_text_color(timeLabel, lv_color_hex(0xffffff), 0);
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, 40);
    
    // Create step counter
    lv_obj_t *stepLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(stepLabel, "Steps: 0");
    lv_label_set_text_color(stepLabel, lv_color_hex(0x00ff00), 0);
    lv_obj_align(stepLabel, LV_ALIGN_BOTTOM_LEFT, 20, 20);
    
    // Create battery indicator
    batteryLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(batteryLabel, getBatteryText());
    lv_label_set_text_color(batteryLabel, getBatteryColor(), 0);
    lv_obj_align(batteryLabel, LV_ALIGN_TOP_RIGHT, -10, 10);
    
    // Center all elements
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, 0);
}

void setupGestures() {
    // Touch interrupt will be handled by LilyGoLib
    // We'll handle touch logic in loop()
}

void applyClockOptimizations() {
    // Use balanced power profile for good UX
    watch.setPowerProfile(POWER_PROFILE_BALANCED);
    watch.setBrightness(clock.brightness);
    
    // Disable non-essential peripherals
    watch.powerIoctl(WATCH_POWER_RADIO, false);
    watch.powerIoctl(WATCH_POWER_GPS, false);
    watch.powerIoctl(WATCH_POWER_DRV2605, false);
}

void loop() {
    static unsigned long lastUpdate = 0;
    static unsigned long lastTouchTime = 0;
    
    // Handle touch interactions
    handleTouchInput();
    
    // Update at reasonable rate (10Hz for smooth UX)
    if (millis() - lastUpdate > 100) {
        updateTimeDisplay();
        updateStepCounter();
        updateBatteryStatus();
        lastUpdate = millis();
    }
    
    // Process display
    lv_task_handler();
    delay(10); // 10Hz refresh
}

void handleTouchInput() {
    if (watch.getTouched()) {
        clock.lastInteraction = millis();
        clock.displayOn = true;
        lastTouchTime = millis();
        
        static unsigned long touchStartTime = millis();
        
        // Detect touch duration for different actions
        if (millis() - touchStartTime < 200) {
            // Short tap - toggle displays
            toggleDisplayStates();
        } else if (millis() - touchStartTime < 1000) {
            // Medium press - switch modes
            switchClockMode((ClockMode)((clock.currentMode + 1) % 4));
            touchStartTime = millis();
        } else {
            // Long press - adjust brightness
            adjustBrightnessByTouchDuration(millis() - touchStartTime);
        }
    }
    }
}

void toggleDisplayStates() {
    // Cycle through display states
    static uint8_t state = 0;
    
    state = (state + 1) % 4;
    
    switch (state) {
        case 0: // All on
            clock.showDate = true;
            clock.showBatteryLevel = true;
            clock.showTemperature = true;
            break;
        case 1: // Time only
            clock.showDate = false;
            clock.showBatteryLevel = false;
            clock.showTemperature = false;
            break;
        case 2: // Battery only
            clock.showDate = false;
            clock.showBatteryLevel = true;
            clock.showTemperature = false;
            clock.displayOn = false; // Keep display on for battery visualization
            break;
        case 3: // Step counter only
            clock.showDate = false;
            clock.showBatteryLevel = false;
            clock.showTemperature = false;
            break;
    }
        
    clock.currentMode = (ClockMode)state;
    
    // Visual feedback
    showVisualFeedback(state);
}

void adjustBrightnessByTouchDuration(unsigned long duration) {
    // Adjust brightness based on how long touch was held
    int8_t change = 0;
    
    if (duration < 200) {
        change = 10; // Quick tap
    } else if (duration < 500) {
        change = -10; // Medium press
    } else if (duration < 1000) {
        change = -20; // Long press
    } else if (duration < 3000) {
        change = -30; // Very long press
    } else {
        change = -40; // Very long press
    }
    }
    
    if (change != 0) {
        adjustBrightness(change);
    }
}

void adjustBrightness(int change) {
    int newBrightness = clock.brightness + change;
    newBrightness = max(10, min(255, newBrightness));
    
    clock.brightness = newBrightness;
    watch.setBrightness(newBrightness);
    
    // Brief visual feedback
    lv_label_set_text(brightnessLabel, getBrightnessText());
    delay(100);
}

void showVisualFeedback(uint8_t state) {
    // Brief flash for visual confirmation
    switch (state) {
        case 0:
            watch.setBrightness(255);
            delay(50);
            watch.setBrightness(clock.brightness);
            delay(50);
            break;
        case 1:
            watch.setBrightness(255);
            delay(100);
            delay(100);
            break;
        case 2:
            watch.setBrightness(255);
            delay(100);
            delay(100);
            break;
        case 3:
            watch.setBrightness(100);
            delay(100);
            delay(100);
            break;
    }
    }
}

void updateTimeDisplay() {
    RTC_Date date;
    RTC_Time time;
    if (watch.getDateTime(&date, &time)) {
        char timeStr[20];
        
        if (clock.is24HourFormat) {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", 
                     time.hour, time.minute, time.second);
        } else {
            int hour12 = time.hour % 12;
            const char *ampm = hour12 >= 12 ? "PM" : "AM";
            snprintf(timeStr, sizeof(timeStr), "%d:%02d:%02d %s", 
                     hour12, time.minute, time.second, ampm);
        }
        
        lv_label_set_text(timeLabel, timeStr);
    } else {
            lv_label_set_text(timeLabel, "Time unavailable");
        }
    }
    
    if (clock.showDate) {
        dateLabel = lv_label_create(lv_scr_act());
        char dateStr[20];
        const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dec"};
        snprintf(dateStr, sizeof(dateStr), "%s %d, %d, %d", 
                 months[date.month - 1], date.day, date.year);
        lv_label_set_text(dateLabel, dateStr);
        lv_obj_align(dateLabel, LV_ALIGN_CENTER, 0, 60);
        lv_obj_show(dateLabel);
        lv_obj_set_style_text_color(dateLabel, lv_color_hex(0xaaaaaa), 0);
    } else {
        lv_obj_hide(dateLabel);
        }
    }
}

void updateStepCounter() {
    uint32_t currentSteps = watch.getStepCounter();
    
    if (currentSteps != clock.lastStepCount) {
        clock.lastStepCount = currentSteps;
        lv_label_set_text(stepLabel, "Steps: %lu", currentSteps);
        
        // Subtle animation on step count change
        static uint8_t previousSteps = 0;
        
        // Pulse effect on count change
        if (previousSteps > 0 && currentSteps > previousSteps) {
            lv_label_set_style_text_color(stepLabel, lv_color_hex(0x00ff00), 0);
        } else {
            lv_label_set_style_text_color(stepLabel, lv_color_hex(0xffffff), 0);
        }
        
        previousSteps = currentSteps;
    }
}

void updateBatteryStatus() {
    clock.batteryVoltage = watch.getBattVoltage();
    clock.batteryPercent = calculateBatteryPercent(clock.batteryVoltage);
    
    lv_label_set_text(batteryLabel, getBatteryText());
    lv_color_t batteryColor = getBatteryColor();
    lv_label_set_text_color(batteryLabel, batteryColor);
}

void switchClockMode(ClockMode mode) {
    clock.currentMode = mode;
    
    switch (mode) {
        case CLOCK_SIMPLE_ANALOG:
            Serial.println("🕐 Switching to Analog Clock");
            lv_label_set_text(modeLabel, "🕐 Analog");
            break;
        case CLOCK_SIMPLE_DIGITAL:
            Serial.println("🔢 Switching to Digital Clock");
            lv_label_set_text(modeLabel, "🔢 Digital");
            break;
        case CLOCK_BATTERY_OPTIMIZED:
            Serial.println("🔋 Switching to Battery Focused");
            lv_label_set_text(modeLabel, "🔋 Battery");
            break;
    }
    }
    
    // Update display elements
    updateDisplayElements();
}

void updateDisplayElements() {
    // Show/hide elements based on mode
    bool showTime = (clock.currentMode != CLOCK_BATTERY_OPTIMIZED);
    bool showStep = (clock.currentMode == CLOCK_SIMPLE_ANALOG || clock.currentMode == CLOCK_SIMPLE_DIGITAL);
    bool showDate = (clock.showDate && clock.currentMode != CLOCK_BATTERY_OPTIMIZED);
    bool showBattery = (clock.showBatteryLevel || clock.currentMode == CLOCK_BATTERY_OPTIMIZED);
    bool showTemperature = (clock.showTemperature && clock.currentMode != CLOCK_BATTERY_OPTIMIZED);
    
    // Hide date in battery mode
    if (clock.showDate) {
        lv_obj_show(dateLabel);
    } else {
        lv_obj_hide(dateLabel);
    }
    
    // Hide step counter in analog mode
    if (!showStep) {
        lv_obj_hide(stepLabel);
    } else {
        lv_obj_show(stepLabel);
    }
    
    // Hide temperature in most modes except battery mode
    if (!showTemperature) {
        lv_obj_hide(temperatureLabel);
    }
    
    // Hide brightness in battery mode
    if (!showBattery) {
        lv_obj_hide(brightnessLabel);
    } else {
        lv_obj_show(brightnessLabel);
    }
}

uint8_t calculateBatteryPercent(uint16_t voltage) {
    if (voltage >= 4200) return 100;
    if (voltage <= 3300) return 0;
    return (uint8_t)(((voltage - 3300) * 100) / 900);
}

String getBatteryText() {
    return String(clock.batteryPercent) + "%";
}

lv_color_t getBatteryColor() {
    if (clock.batteryPercent > 60) {
        return BATTERY_GOOD;
    } else if (clock.batteryPercent > 30) {
        return BATTERY_PERFECT;
    } else if (clock.batteryPercent > 15) {
        return BATTERY_LOW;
    } else {
        return BATTERY_CRITICAL;
    }
}

String getBrightnessText() {
    return String(clock.brightness) + "%";
}