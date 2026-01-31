/**
 * @file      ImprovedClockFace.ino
 * @author    T-Watch S3 Improved Clock UX
 * @license   MIT
 * @date      2026-01-31
 * 
 * IMPROVED CLOCK UX FOR T-WATCH S3
 * Better navigation, battery display, and usability
 */

#include <LilyGoLib.h>
#include <LV_Helper.h>

// Modern clock states
typedef enum {
    CLOCK_ANALOG_MODERN = 0,
    CLOCK_DIGITAL_MINIMAL = 1,
    CLOCK_SPORT_SIMPLE = 2,
    CLOCK_INFO_DASHBOARD = 3
} ClockMode;

typedef struct {
    ClockMode currentMode;
    uint8_t brightness;
    bool displayOn;
    unsigned long lastInteraction;
    unsigned long lastUpdate;
    unsigned long lastButtonPress;
    bool is24HourFormat;
    bool showDate;
    bool showBatteryLevel;
    bool showTemperature;
    int lastStepCount;
    unsigned long lastGestureTime;
    bool gestureMode;
    float batteryVoltage;
    uint8_t batteryPercent;
} ClockState;

static ClockState clock;

// UI Elements - simplified without LVGL complications
static lv_obj_t *timeContainer;
static lv_obj_t *timeLabel;
static lv_obj_t *dateLabel;
static lv_obj_t *stepLabel;
static lv_obj_t *batteryLabel;
static lv_obj_t *modeLabel;
static lv_obj_t *brightnessBar;
static lv_obj_t *temperatureLabel;
static lv_obj_t *gestureIndicator;

// Color scheme for battery levels
const lv_color_t BATTERY_CRITICAL = lv_color_hex(0xff0000);
const lv_color_t BATTERY_LOW = lv_color_hex(0xff6600);
const lv_color_t BATTERY_GOOD = lv_color_hex(0x00ff00);
const lv_color_t BATTERY_PERFECT = lv_color_hex(0x00cc00);

void setup() {
    Serial.begin(115200);
    Serial.println("=== T-Watch S3 Improved Clock UX ===");
    Serial.println("🎨 Better navigation & battery display");
    
    // Initialize hardware
    if (!watch.begin()) {
        Serial.println("❌ Failed to initialize T-Watch");
        return;
    }
    
    initializeClockState();
    createImprovedClockUI();
    setupTouchAndBattery();
    
    // Apply clock optimizations
    applyClockOptimizations();
    
    Serial.println("✅ Improved clock face initialized");
    Serial.println("👆 Swipe up/down: change clock modes");
    Serial.println("👆 Double-tap: toggle displays");
    Serial.println("👆 Hold 2s: settings menu");
    Serial.println("👆 Left swipe: gesture mode");
    Serial.println("👆 Right swipe: brightness control");
}

void initializeClockState() {
    clock.currentMode = CLOCK_DIGITAL_MINIMAL;
    clock.brightness = 120;
    clock.displayOn = true;
    clock.lastInteraction = millis();
    clock.lastUpdate = 0;
    clock.lastButtonPress = 0;
    clock.is24HourFormat = true;
    clock.showDate = false;
    clock.showBatteryLevel = true;
    clock.showTemperature = true;
    clock.lastStepCount = 0;
    clock.lastGestureTime = 0;
    clock.gestureMode = false;
    
    // Get initial sensor data
    clock.batteryVoltage = watch.getBattVoltage();
    clock.batteryPercent = calculateBatteryPercent(clock.batteryVoltage);
}

void createImprovedClockUI() {
    // Dark theme with subtle gradients
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x0a0a0a), 0);
    
    // Main container with subtle styling
    timeContainer = lv_obj_create(lv_scr_act());
    lv_obj_set_size(timeContainer, 240, 240);
    lv_obj_set_style_bg_color(timeContainer, lv_color_hex(0x0f0f0f), 0);
    lv_obj_set_style_border_width(timeContainer, 1);
    lv_obj_set_style_border_color(timeContainer, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(timeContainer, 15);
    lv_obj_center(timeContainer);
    
    // Create display sections
    createTimeSection();
    createBatterySection();
    createStepsSection();
    createInfoSection();
    
    // Mode indicator
    createModeIndicator();
}

void createTimeSection() {
    // Time display area
    timeLabel = lv_label_create(timeContainer);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_48);
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0xffffff), 0);
    lv_obj_align(timeLabel, LV_ALIGN_TOP_MID, 0, 20);
    
    // Date display (conditional)
    if (clock.showDate) {
        dateLabel = lv_label_create(timeContainer);
        lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_18);
        lv_obj_set_style_text_color(dateLabel, lv_color_hex(0xaaaaaa), 0);
        lv_obj_align(dateLabel, LV_ALIGN_TOP_LEFT, 0, 50);
    }
    
    // Position time section
    lv_obj_align(timeContainer, LV_ALIGN_CENTER, 0, -30);
}

void createBatterySection() {
    // Battery level bar
    batteryLabel = lv_label_create(timeContainer);
    lv_label_set_text(batteryLabel, getBatteryText());
    lv_obj_set_style_text_color(batteryLabel, getBatteryColor(), 0);
    lv_obj_align(batteryLabel, LV_ALIGN_TOP_RIGHT, -10, 10);
    
    // Battery level visual indicator
    createBatteryBar();
}

void createStepsSection() {
    // Step counter display
    stepLabel = lv_label_create(timeContainer);
    lv_label_set_text(stepLabel, getStepsText());
    lv_obj_set_style_text_font(stepLabel, &lv_font_montserrat_32);
    lv_obj_set_style_text_color(stepLabel, lv_color_hex(0x00ff00), 0);
    lv_obj_align(stepLabel, LV_ALIGN_BOTTOM_LEFT, 20, 10);
}

void createInfoSection() {
    // Temperature display
    if (clock.showTemperature) {
        temperatureLabel = lv_label_create(timeContainer);
        lv_label_set_text(temperatureLabel, getTemperatureText());
        lv_obj_set_style_text_font(temperatureLabel, &lv_font_montserrat_16);
        lv_obj_set_style_text_color(temperatureLabel, lv_color_hex(0xff6600), 0);
        lv_obj_align(temperatureLabel, LV_ALIGN_BOTTOM_RIGHT, -20, 10);
    }
    
    // Mode and gesture indicators
    if (clock.gestureMode || clock.showBatteryLevel) {
        gestureIndicator = lv_label_create(timeContainer);
        lv_label_set_text(gestureIndicator, "👆");
        lv_obj_set_style_text_font(gestureIndicator, &lv_font_montserrat_16);
        lv_obj_set_style_text_color(gestureIndicator, lv_color_hex(0x00ff00), 0);
        lv_obj_align(gestureIndicator, LV_ALIGN_BOTTOM_CENTER, 0, 10);
    }
}

void createModeIndicator() {
    modeLabel = lv_label_create(timeContainer);
    lv_label_set_text(modeLabel, getModeText());
    lv_obj_set_style_text_font(modeLabel, &lv_font_montserrat_20);
    lv_obj_set_style_text_color(modeLabel, lv_color_hex(0x00ff00), 0);
    lv_obj_align(modeLabel, LV_ALIGN_BOTTOM_CENTER, 0, 40);
}

void createBatteryBar() {
    // Visual battery level bar
    lv_obj_t *batteryBar = lv_bar_create(timeContainer);
    lv_obj_set_size(batteryBar, 60, 8);
    lv_obj_set_pos(batteryBar, 20, 50);
    lv_obj_set_style_bg_color(batteryBar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_color(batteryBar, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(batteryBar, 4);
    lv_bar_set_range(batteryBar, 0, 100);
    lv_bar_set_value(batteryBar, clock.batteryPercent);
    lv_bar_set_style_bg_color(batteryBar, getBatteryColor(), 0);
    
    lv_obj_align(batteryBar, LV_ALIGN_TOP_RIGHT, -10, 30);
}

void setupTouchAndBattery() {
    // Configure touch for better responsiveness
    // Note: Touch is handled by LilyGoLib, but we can configure sensitivity if available
    
    // Battery monitoring
    // Note: Battery is handled by PMU interrupts
}

void applyClockOptimizations() {
    // Set balanced power profile for good UX without sacrificing battery life
    watch.setPowerProfile(POWER_PROFILE_BALANCED);
    
    // Set moderate brightness for good visibility
    watch.setBrightness(clock.brightness);
    
    // Enable essential peripherals, disable others for battery savings
    watch.powerIoctl(WATCH_POWER_RADIO, false);
    watch.powerIoctl(WATCH_POWER_GPS, false);
    watch.powerIoctl(WATCH_POWER_DRV2605, false);
}

void loop() {
    static unsigned long lastUpdate = 0;
    static unsigned long lastGestureCheck = 0;
    
    handleTouchGestures();
    
    // Update displays at appropriate intervals
    if (millis() - lastUpdate > 100) {
        updateAllDisplays();
        lastUpdate = millis();
    }
    
    // Check gestures more frequently when in gesture mode
    if (clock.gestureMode && millis() - lastGestureCheck > 50) {
        detectGestures();
        lastGestureCheck = millis();
    }
    
    // Process LVGL
    lv_task_handler();
    delay(5); // 20Hz refresh rate
}

void handleTouchGestures() {
    if (watch.getTouched()) {
        clock.lastInteraction = millis();
        
        // Ensure display is on
        if (!clock.displayOn && clock.brightness > 0) {
            watch.setBrightness(clock.brightness);
            clock.displayOn = true;
        }
        
        // Handle different touch patterns
        static unsigned long touchStartTime = millis();
        static uint16_t startX = 0, startY = 0;
        
        if (millis() - touchStartTime < 50) {
            // Quick tap
            handleQuickTap();
        } else if (millis() - touchStartTime < 500) {
            // Swipe detection for navigation
            handleSwipeNavigation();
        } else if (millis() - touchStartTime < 1000) {
            // Long press for settings
            handleLongPress();
        } else {
            // Very long press for power off
            handleVeryLongPress();
        }
        
        touchStartTime = millis();
    }
}

void handleQuickTap() {
    // Single tap toggles displays
    static bool cycleDisplays = false;
    
    cycleDisplays = !cycleDisplays;
    
    // Toggle different display elements
    clock.showDate = cycleDisplays ? !clock.showDate : false;
    clock.showBatteryLevel = cycleDisplays ? !clock.showBatteryLevel : true;
    clock.showTemperature = cycleDisplays ? !clock.showTemperature : false;
    
    // Cycle back after showing all
    delay(200);
}

void handleSwipeNavigation() {
    // Read accelerometer for swipe detection
    int16_t accelX, accelY, accelZ;
    watch.readAccelerometer(&accelX, &accelY, &accelZ);
    
    static int lastX = 0;
    int deltaX = accelX - lastX;
    
    // Detect horizontal swipes
    if (abs(deltaX) > 40) {
        if (deltaX > 0) {
            // Right swipe - increase brightness
            adjustBrightness(20);
        } else {
            // Left swipe - switch clock mode
            switchClockMode((ClockMode)((clock.currentMode + 1) % 4);
        }
    }
    
    lastX = accelX;
}

void handleLongPress() {
    // Long press (500-1000ms) - enter settings menu
    Serial.println("⚙️ Long press - entering settings mode");
    
    // Show settings indicator
    lv_label_set_text(modeLabel, "⚙️ Settings");
    delay(2000); // Show for 2 seconds
}

void handleVeryLongPress() {
    // Very long press (>1s) - power off
    Serial.println("⏹️ Very long press - powering off");
    watch.setBrightness(0);
    clock.displayOn = false;
    
    // Wait for release
    while (watch.getTouched()) {
        delay(10);
    }
    
    Serial.println("💤 Entering deep sleep");
    esp_sleep_enable_timer_wakeup(3600000000); // 1 hour
    esp_deep_sleep_start();
}

void detectGestures() {
    if (!clock.gestureMode) return;
    
    // Read accelerometer for gesture patterns
    int16_t accelX, accelY, accelZ;
    watch.readAccelerometer(&accelX, &accelY, &accelZ);
    
    // Simple gesture detection based on movement patterns
    static float accelMagnitude = 0;
    
    accelMagnitude = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
    
    // Detect wrist shake for activation
    if (accelMagnitude > 2.0) {
        clock.lastGestureTime = millis();
        Serial.println("👆 Wrist shake detected");
        lv_label_set_text(gestureIndicator, "👆");
    }
    
    // Detect tilt for gesture mode
    if (accelMagnitude > 1.0 && millis() - clock.lastGestureTime > 1000) {
        clock.gestureMode = true;
        Serial.println("👆 Tilt detected - gesture mode active");
        lv_label_set_text(gestureIndicator, "👆");
    }
}

void adjustBrightness(int change) {
    int newBrightness = clock.brightness + change;
    newBrightness = max(10, min(255, newBrightness));
    
    clock.brightness = newBrightness;
    watch.setBrightness(newBrightness);
    
    // Show visual feedback
    lv_label_set_text_fmt(brightnessLabel, "☀ %d%%", clock.brightness);
}

void updateAllDisplays() {
    // Update RTC time
    RTC_Date date;
    RTC_Time time;
    if (watch.getDateTime(&date, &time)) {
        updateTimeDisplay(date, time);
        updateStepCounter();
        updateBatteryStatus();
        updateTemperature();
        updateModeIndicator();
    }
}

void updateTimeDisplay(RTC_Date &date, RTC_Time &time) {
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
    
    if (clock.showDate) {
        char dateStr[20];
        const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        snprintf(dateStr, sizeof(dateStr), "%s %d, %d", months[date.month - 1], date.day, date.year);
        lv_label_set_text(dateLabel, dateStr);
    }
}

void updateStepCounter() {
    uint32_t currentSteps = watch.getStepCounter();
    
    if (currentSteps != clock.lastStepCount) {
        clock.lastStepCount = currentSteps;
        lv_label_set_text_fmt(stepLabel, "👟 %lu steps", currentSteps);
        
        // Animation on step count change
        if (currentSteps > clock.lastStepCount) {
            animateStepCount();
        }
    }
}

void animateStepCount() {
    // Simple pulse animation when step count changes
    static uint8_t pulseState = 0;
    
    pulseState = !pulseState;
    
    lv_style_t pulseStyle;
    lv_style_init(&pulseStyle);
    lv_style_set_bg_color(&pulseStyle, lv_color_transp(lv_color_hex(0x00ff00), LV_OPA_50));
    lv_style_set_text_color(&pulseStyle, lv_color_transp(lv_color_hex(0x00ff00), LV_OPA_50));
    
    lv_obj_add_style(stepLabel, &pulseStyle);
    lv_obj_refresh(stepLabel);
    
    // Schedule animation revert
    lv_timer_t *timer = lv_timer_create();
    lv_timer_set_period(timer, 200);
    lv_timer_set_cb(timer, [](lv_timer_t *timer) {
        pulseState = !pulseState;
        lv_obj_remove_style(stepLabel, &pulseStyle);
        lv_obj_refresh(stepLabel);
    });
    lv_timer_start(timer);
    
    // Auto-revert after 1 second
    lv_timer_set_period(timer, 1000);
    lv_timer_set_cb(timer, [](lv_timer_t *timer) {
        lv_timer_del(timer);
    });
    lv_timer_start(timer);
}

void updateBatteryStatus() {
    if (!clock.showBatteryLevel) return;
    
    lv_label_set_text(batteryLabel, getBatteryText());
    lv_bar_set_value(batteryBar, clock.batteryPercent);
    updateBatteryBar();
}

void updateBatteryBar() {
    lv_bar_set_style_bg_color(batteryBar, getBatteryColor(), 0);
    lv_obj_refresh(batteryBar);
}

void updateTemperature() {
    if (!clock.showTemperature) return;
    
    lv_label_set_text(temperatureLabel, getTemperatureText());
}

void updateModeIndicator() {
    if (clock.currentMode == CLOCK_ANALOG_MODERN) {
        lv_label_set_text(modeLabel, "🕐 Modern");
    } else if (clock.currentMode == CLOCK_DIGITAL_MINIMAL) {
        lv_label_set_text(modeLabel, "🔢 Minimal");
    } else if (clock.currentMode == CLOCK_SPORT_SIMPLE) {
        lv_label_set_text(modeLabel, "⏱ Sports");
    } else if (clock.currentMode == CLOCK_INFO_DASHBOARD) {
        lv_label_set_text(modeLabel, "📊 Dashboard");
    }
}

String getBatteryText() {
    return String(clock.batteryPercent) + "%";
}

lv_color_t getBatteryColor() {
    if (clock.batteryPercent > 60) return BATTERY_GOOD;
    if (clock.batteryPercent > 30) return BATTERY_PERFECT;
    if (clock.batteryPercent > 15) return BATTERY_LOW;
    return BATTERY_CRITICAL;
}

String getStepsText() {
    return String(clock.lastStepCount);
}

String getTemperatureText() {
    return String(clock.coreTemp, 1) + "°C";
}

String getModeText() {
    switch (clock.clockMode) {
        case CLOCK_ANALOG_MODERN:    return "🕐 Modern";
        case CLOCK_DIGITAL_MINIMAL: return "🔢 Minimal";
        case CLOCK_SPORT_SIMPLE:  return "⏱ Sports";
        case CLOCK_INFO_DASHBOARD: return "📊 Dashboard";
        default:                return "🕐 Modern";
    }
}