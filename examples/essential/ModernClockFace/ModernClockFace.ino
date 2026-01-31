/**
 * @file      ModernClockFace.ino
 * @author    T-Watch S3 Modern Clock Face
 * @license   MIT
 * @date      2026-01-31
 * 
 * MODERN SMARTWATCH CLOCK FACE
 * Features: battery optimization, gesture controls, multiple watch faces, smooth animations
 */

#include <LilyGoLib.h>
#include <LV_Helper.h>

// Clock face modes
enum ClockFace {
    ANALOG_CLASSIC = 0,
    MINIMAL_DIGITAL = 1,
    SPORT_CHRONOGRAPH = 2,
    SMART_NOTIFICATIONS = 3
    BATTERY_FOCUSED = 4,
    CUSTOM_FACE_COUNT = 5
};

// UI state management
typedef struct {
    ClockFace currentFace;
    uint8_t brightness;
    bool displayOn;
    unsigned long lastInteraction;
    unsigned long lastAnimationUpdate;
    bool is24HourFormat;
    bool showSeconds;
    bool showDate;
    uint8_t gestureMode;
    int lastStepCount;
    float batteryVoltage;
    uint8_t batteryPercent;
    float coreTemp;
} UIState;

// Gesture recognition
typedef struct {
    uint16_t accelX, accelY, accelZ;
    unsigned long lastGestureTime;
    bool isInGestureMode;
} GestureState;

// Global UI elements
static lv_obj_t *mainContainer;
static lv_obj_t *clockDisplay;
static lv_obj_t *batteryIcon;
static lv_obj_t *timeLabel;
static lv_obj_t *dateLabel;
static lv_obj_t *stepLabel;
static lv_obj_t *tempLabel;
static lv_obj_t *gestureIndicator;
static lv_obj_t *modeButton;
static lv_obj_t *brightnessSlider;

// Animation objects
static lv_anim_t slideAnimation;
static lv_obj_t *notificationOverlay;
static lv_obj_t *faceTransition;

static UIState ui;
static GestureState gesture;

void setup() {
    Serial.begin(115200);
    Serial.println("=== T-Watch Modern Clock Face ===");
    Serial.println("🎨 Modern UX with gesture controls");
    
    // Initialize hardware
    watch.begin();
    beginLvglHelper();
    
    // Initialize UI state
    initializeUIState();
    
    // Create modern clock UI
    createModernClockUI();
    
    // Attach touch and gesture handlers
    setupTouchAndGestures();
    
    // Apply battery optimization
    applyClockOptimizations();
    
    Serial.println("✅ Modern clock face initialized");
    Serial.println("👆 Swipe up/down to change faces");
    Serial.println("👆 Double-tap for settings");
    Serial.println("👆 Left swipe for brightness");
    Serial.println("👆 Right swipe for gesture mode");
}

void initializeUIState() {
    // Set initial state
    ui.currentFace = ANALOG_CLASSIC;
    ui.brightness = 80;
    ui.displayOn = true;
    ui.lastInteraction = millis();
    ui.lastAnimationUpdate = 0;
    ui.is24HourFormat = true;
    ui.showSeconds = true;
    ui.showDate = true;
    ui.gestureMode = 0;
    ui.lastStepCount = 0;
    
    // Initialize sensor data
    ui.batteryVoltage = watch.getBattVoltage();
    ui.batteryPercent = calculateBatteryPercent(ui.batteryVoltage);
    ui.coreTemp = watch.readAccelTemp();
    
    // Initialize gesture state
    gesture.accelX = 0;
    gesture.accelY = 0;
    gesture.accelZ = 0;
    gesture.lastGestureTime = 0;
    gesture.isInGestureMode = false;
}

void createModernClockUI() {
    // Create dark background
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    
    // Main container
    mainContainer = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mainContainer, 240, 240);
    lv_obj_set_style_bg_color(mainContainer, lv_color_hex(0x0a0a0a), 0);
    lv_obj_set_style_border_width(mainContainer, 2);
    lv_obj_set_style_border_color(mainContainer, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(mainContainer, 20);
    lv_obj_center(mainContainer);
    
    // Clock display area
    clockDisplay = lv_obj_create(mainContainer);
    lv_obj_set_size(clockDisplay, 220, 180);
    lv_obj_set_style_bg_color(clockDisplay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(clockDisplay, 15);
    lv_obj_align(clockDisplay, LV_ALIGN_CENTER, 0, 10);
    
    // Top bar with battery and mode button
    createTopBar();
    
    // Main time display
    createTimeDisplay();
    
    // Bottom bar with step counter and temperature
    createBottomBar();
    
    // Initialize current clock face
    switchClockFace(ui.currentFace);
}

void createTopBar() {
    // Battery indicator with icon
    batteryIcon = lv_obj_create(mainContainer);
    lv_obj_set_size(batteryIcon, 30, 20);
    lv_obj_align(batteryIcon, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_bg_color(batteryIcon, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(batteryIcon, 10);
    
    // Mode selection button
    modeButton = lv_btn_create(mainContainer);
    lv_obj_set_size(modeButton, 60, 30);
    lv_obj_align(modeButton, LV_ALIGN_TOP_LEFT, 10, 10);
    lv_obj_set_style_bg_color(modeButton, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_radius(modeButton, 15);
    lv_obj_set_style_text_color(modeButton, lv_color_white(), 0);
    
    lv_obj_t *buttonLabel = lv_label_create(modeButton);
    lv_label_set_text(buttonLabel, "Face");
    lv_obj_center(buttonLabel);
}

void createTimeDisplay() {
    timeLabel = lv_label_create(clockDisplay);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_48);
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0x00ff00), 0);
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, -20);
}

void createBottomBar() {
    // Step counter
    stepLabel = lv_label_create(clockDisplay);
    lv_obj_set_style_text_font(stepLabel, &lv_font_montserrat_24);
    lv_obj_set_style_text_color(stepLabel, lv_color_hex(0x00ffff), 0);
    lv_obj_align(stepLabel, LV_ALIGN_BOTTOM_LEFT, 20, 10);
    
    // Temperature display
    tempLabel = lv_label_create(clockDisplay);
    lv_obj_set_style_text_font(tempLabel, &lv_font_montserrat_20);
    lv_obj_set_style_text_color(tempLabel, lv_color_hex(0xff6600), 0);
    lv_obj_align(tempLabel, LV_ALIGN_BOTTOM_RIGHT, -20, 10);
    
    // Date display
    dateLabel = lv_label_create(clockDisplay);
    lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_18);
    lv_obj_set_style_text_color(dateLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(dateLabel, LV_ALIGN_BOTTOM_CENTER, 0, -30);
}

void setupTouchAndGestures() {
    // Attach touch handler for interaction
    // NOTE: Touch interrupts are handled in LilyGoLib
    
    // Initialize gesture recognition
    gesture.lastGestureTime = millis();
}

void applyClockOptimizations() {
    // Set balanced power profile for good UX
    watch.setPowerProfile(POWER_PROFILE_BALANCED);
    
    // Set moderate brightness for good visibility
    watch.setBrightness(ui.brightness);
    
    // Disable unused peripherals to save power
    watch.powerIoctl(WATCH_POWER_RADIO, false);
    watch.powerIoctl(WATCH_POWER_GPS, false);
    watch.powerIoctl(WATCH_POWER_DRV2605, false);
}

void switchClockFace(ClockFace face) {
    switch (face) {
        case ANALOG_CLASSIC:
            createAnalogClockFace();
            break;
        case MINIMAL_DIGITAL:
            createDigitalClockFace();
            break;
        case SPORT_CHRONOGRAPH:
            createSportChronographFace();
            break;
        case SMART_NOTIFICATIONS:
            createNotificationFace();
            break;
        case BATTERY_FOCUSED:
            createBatteryFace();
            break;
    }
}

void createAnalogClockFace() {
    Serial.println("🕐 Switching to Analog Classic Clock");
    
    // Clear previous content
    lv_obj_clean(clockDisplay);
    
    // Create analog clock canvas
    lv_obj_t *canvas = lv_canvas_create(clockDisplay);
    lv_obj_set_size(canvas, 200, 200);
    lv_obj_center(canvas);
    
    // Set canvas as drawing area
    static lv_color_t canvas_buf[200 * 200];
    lv_canvas_set_buffer(canvas, canvas_buf, 200, 200, LV_IMG_CF_TRUE_COLOR);
    
    // Clear canvas with black
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
    
    // Draw clock circle
    drawAnalogClockCircle(canvas);
    
    // Draw hour markers
    drawHourMarkers(canvas);
    
    // Draw hour and minute hands
    updateAnalogHands(canvas);
}

void createDigitalClockFace() {
    Serial.println("🔢 Switching to Minimal Digital Clock");
    
    // Clear and setup for digital display
    lv_obj_clean(clockDisplay);
    
    // Create large digital time display
    lv_obj_t *digitalTime = lv_label_create(clockDisplay);
    lv_obj_set_style_text_font(digitalTime, &lv_font_montserrat_72);
    lv_obj_set_style_text_color(digitalTime, lv_color_hex(0x00ff00), 0);
    lv_obj_set_style_bg_opa(digitalTime, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_color(digitalTime, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(digitalTime, 1);
    lv_obj_set_style_border_color(digitalTime, lv_color_hex(0x444444), 0);
    lv_obj_set_style_radius(digitalTime, 5);
    lv_obj_center(digitalTime);
    
    // Store reference for updates
    ui.currentFace = MINIMAL_DIGITAL;
}

void createSportChronographFace() {
    Serial.println("⏱ Switching to Sport Chronograph");
    
    // Clear for chronograph
    lv_obj_clean(clockDisplay);
    
    // Create stopwatch-style display
    lv_obj_t *stopwatchDisplay = lv_label_create(clockDisplay);
    lv_label_set_text(stopwatchDisplay, "00:00:00");
    lv_label_set_style_text_font(stopwatchDisplay, &lv_font_montserrat_48);
    lv_obj_set_style_text_color(stopwatchDisplay, lv_color_hex(0x00ff00), 0);
    lv_obj_align(stopwatchDisplay, LV_ALIGN_CENTER, 0, 0);
    
    ui.currentFace = SPORT_CHRONOGRAPH;
}

void createNotificationFace() {
    Serial.println("🔔 Switching to Smart Notifications");
    
    // Clear for notifications
    lv_obj_clean(clockDisplay);
    
    // Create notification area
    notificationOverlay = lv_obj_create(clockDisplay);
    lv_obj_set_size(notificationOverlay, 220, 120);
    lv_obj_set_style_bg_color(notificationOverlay, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_border_width(notificationOverlay, 1);
    lv_obj_set_style_border_color(notificationOverlay, lv_color_hex(0x333333), 0);
    lv_obj_set_radius(notificationOverlay, 15);
    lv_obj_align(notificationOverlay, LV_ALIGN_CENTER, 0, 20);
    
    // Example notification
    lv_obj_t *notifLabel = lv_label_create(notificationOverlay);
    lv_label_set_text(notifLabel, "📱 New Message");
    lv_obj_set_style_text_font(notifLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(notifLabel, lv_color_white(), 0);
    lv_obj_center(notifLabel);
    
    ui.currentFace = SMART_NOTIFICATIONS;
}

void createBatteryFace() {
    Serial.println("🔋 Switching to Battery Focused View");
    
    // Clear for battery display
    lv_obj_clean(clockDisplay);
    
    // Create large battery indicator
    createBatteryIndicator();
    
    // Show battery percentage prominently
    lv_obj_t *batteryPercentLabel = lv_label_create(clockDisplay);
    lv_label_set_text(batteryPercentLabel, "100%");
    lv_obj_set_style_text_font(batteryPercentLabel, &lv_font_montserrat_72, 0);
    lv_obj_set_style_text_color(batteryPercentLabel, lv_color_hex(0x00ff00), 0);
    lv_obj_align(batteryPercentLabel, LV_ALIGN_CENTER, 0, 20);
    
    // Add usage tips
    lv_obj_t *tipsLabel = lv_label_create(clockDisplay);
    lv_label_set_text(tipsLabel, "💡 Reduce brightness for longer life");
    lv_obj_set_style_text_font(tipsLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(tipsLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(tipsLabel, LV_ALIGN_BOTTOM_CENTER, 0, -30);
    
    ui.currentFace = BATTERY_FOCUSED;
}

void createBatteryIndicator() {
    // Create battery visualization for battery focused face
    lv_obj_t *batteryCanvas = lv_canvas_create(clockDisplay);
    lv_obj_set_size(batteryCanvas, 120, 60);
    lv_obj_align(batteryCanvas, LV_ALIGN_CENTER, 0, -30);
    
    static lv_color_t battery_buf[120 * 60];
    lv_canvas_set_buffer(batteryCanvas, battery_buf, 120, 60, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_fill_bg(batteryCanvas, lv_color_hex(0x000000), LV_OPA_COVER);
}

void drawAnalogClockCircle(lv_obj_t *canvas) {
    lv_canvas_draw_arc(canvas, 100, 100, 80, lv_color_hex(0x00ff00), 0);
    lv_canvas_draw_arc(canvas, 100, 100, 90, lv_color_hex(0x00ff00), 0);
    lv_canvas_draw_arc(canvas, 100, 100, 60, lv_color_hex(0x00ff00), 0);
    lv_canvas_draw_arc(canvas, 100, 100, 30, lv_color_hex(0x00ff00), 0);
}

void drawHourMarkers(lv_obj_t *canvas) {
    
    for (int hour = 0; hour < 12; hour++) {
        int angle = hour * 30; // 30 degrees per hour
        int x1 = 100 + 70 * cos((angle - 90) * PI / 180.0);
        int y1 = 100 + 70 * sin((angle - 90) * PI / 180.0);
        int x2 = 100 + 85 * cos(angle * PI / 180.0);
        int y2 = 100 + 85 * sin(angle * PI / 180.0);
        
        // Draw hour marker
        lv_canvas_draw_line(canvas, x1, y1, x2, y2, lv_color_hex(0xffffff), 2);
        
        // Draw hour number
        char hourStr[3];
        snprintf(hourStr, sizeof(hourStr), "%d", hour == 0 ? 12 : hour);
        
        int textX = 100 + 50 * cos(angle * PI / 180.0) - 8;
        int textY = 100 + 50 * sin(angle * PI / 180.0) + 8;
        
        lv_canvas_draw_text(canvas, hourStr, &lv_font_montserrat_12, textX, textY, lv_color_hex(0xffffff), 0);
    }
}

void updateAnalogHands(lv_obj_t *canvas) {
    // Get current time
    RTC_Date date;
    RTC_Time time;
    if (watch.getDateTime(&date, &time)) {
        // Calculate angles
        float secondAngle = (time.second % 60) * 6; // 6 degrees per second
        float minuteAngle = (time.minute % 60) * 6 + (time.second / 60.0) * 6; // Include seconds
        float hourAngle = ((time.hour % 12) + (time.minute / 60.0)) * 30 + (time.second / 60.0) * 0.5;
        
        // Draw hour hand
        int hourX = 100 + 60 * cos((hourAngle - 90) * PI / 180.0);
        int hourY = 100 + 60 * sin((hourAngle - 90) * PI / 180.0);
        lv_canvas_draw_line(canvas, 100, 100, hourX, hourY, lv_color_hex(0xffffff), 4);
        
        // Draw minute hand
        int minX = 100 + 80 * cos((minuteAngle - 90) * PI / 180.0);
        int minY = 100 + 80 * sin((minuteAngle - 90) * PI / 180.0);
        lv_canvas_draw_line(canvas, 100, 100, minX, minY, lv_color_hex(0xff6600), 2);
        
        // Draw second hand (if enabled)
        if (ui.showSeconds) {
            int secX = 100 + 90 * cos((secondAngle - 90) * PI / 180.0);
            int secY = 100 + 90 * sin((secondAngle - 90) * PI / 180.0);
            lv_canvas_draw_line(canvas, 100, 100, secX, secY, lv_color_hex(0xff0000), 1);
        }
    }
}

void updateClockDisplay() {
    RTC_Date date;
    RTC_Time time;
    
    if (watch.getDateTime(&date, &time)) {
        char timeStr[20];
        
        if (ui.is24HourFormat) {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", time.hour, time.minute, time.second);
        } else {
            int hour12 = time.hour % 12;
            const char *ampm = hour12 >= 12 ? "PM" : "AM";
            snprintf(timeStr, sizeof(timeStr), "%d:%02d:%02d %s", hour12, time.minute, time.second, ampm);
        }
        
        lv_label_set_text(timeLabel, timeStr);
        
        // Update date if enabled
        if (ui.showDate) {
            char dateStr[20];
            const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
            snprintf(dateStr, sizeof(dateStr), "%s %d, %d", months[date.month - 1], date.day, date.year);
            lv_label_set_text(dateLabel, dateStr);
        }
    }
}

void updateBatteryIndicator() {
    
    // Calculate fill percentage
    uint8_t fillPercent = max(0, min(100, ui.batteryPercent));
    
    // Color based on battery level
    lv_color_t batteryColor;
    if (ui.batteryPercent > 60) {
        batteryColor = lv_color_hex(0x00ff00); // Green
    } else if (ui.batteryPercent > 30) {
        batteryColor = lv_color_hex(0xffff00); // Yellow
    } else if (ui.batteryPercent > 15) {
        batteryColor = lv_color_hex(0xff8800); // Orange
    } else {
        batteryColor = lv_color_hex(0xff0000); // Red
    }
    
    // Clear and redraw
    lv_canvas_fill_bg(batteryIcon, lv_color_hex(0x000000), LV_OPA_COVER);
    
    // Draw battery outline
    lv_canvas_draw_rect(batteryIcon, 2, 4, 26, 12, batteryColor);
    
    // Fill battery level
    if (fillPercent > 0) {
        int fillWidth = (22 * fillPercent) / 100;
        lv_canvas_fill_bg(batteryIcon, batteryColor, LV_OPA_COVER);
    }
    
    lv_canvas_inv_buf(batteryIcon);
    lv_canvas_invalidate(batteryIcon);
}

void loop() {
    static unsigned long lastUIUpdate = 0;
    static unsigned long lastGestureCheck = 0;
    static unsigned long lastSensorUpdate = 0;
    static unsigned long lastBatteryUpdate = 0;
    
    handleTouchGestures();
    updateSensorData();
    
    // Update clock display every 100ms
    if (millis() - lastUIUpdate > 100) {
        updateClockDisplay();
        updateBatteryIndicator();
        lastUIUpdate = millis();
    }
    
    // Check gestures every 50ms
    if (millis() - lastGestureCheck > 50) {
        checkGestures();
        lastGestureCheck = millis();
    }
    
    // Update sensor data every second
    if (millis() - lastSensorUpdate > 1000) {
        updateSensorData();
        lastSensorUpdate = millis();
    }
    
    // Update battery every 5 seconds
    if (millis() - lastBatteryUpdate > 5000) {
        ui.batteryVoltage = watch.getBattVoltage();
        ui.batteryPercent = calculateBatteryPercent(ui.batteryVoltage);
        updateBatteryIndicator();
        lastBatteryUpdate = millis();
    }
    
    // Auto-brightness control
    handleAutoBrightness();
    
    // Process LVGL
    lv_task_handler();
    delay(5);
}

void handleTouchGestures() {
    if (watch.getTouched()) {
        ui.lastInteraction = millis();
        ui.displayOn = true;
        
        // Turn on display if it was off
        if (ui.brightness == 0) {
            ui.brightness = 80;
            watch.setBrightness(ui.brightness);
        }
        
        // Short press after 500ms = mode change
        delay(500);
        
        if (watch.getTouched()) {
            // Long press = enter gesture mode
            ui.gestureMode = !ui.gestureMode;
            gesture.lastGestureTime = millis();
            Serial.println("👆 Gesture mode: " + String(ui.gestureMode ? "ON" : "OFF"));
            
            // Show gesture indicator
            if (ui.gestureMode) {
                lv_obj_set_style_bg_color(gestureIndicator, lv_color_hex(0x00ff00), 0);
            } else {
                lv_obj_set_style_bg_color(gestureIndicator, lv_color_hex(0x333333), 0);
            }
        }
    }
}

void checkGestures() {
    if (!ui.gestureMode) return;
    
    // Read accelerometer for gesture recognition
    int16_t accelX, accelY, accelZ;
    watch.readAccelerometer(&accelX, &accelY, &accelZ);
    
    // Simple gesture detection based on acceleration patterns
    detectSwipeGesture(accelX, accelY, accelZ);
    
    gesture.accelX = accelX;
    gesture.accelY = accelY;
    gesture.accelZ = accelZ;
}

void detectSwipeGesture(int16_t x, int16_t y, int16_t z) {
    unsigned long now = millis();
    
    if (now - gesture.lastGestureTime < 200) return; // Debounce
    
    // Detect horizontal swipes
    int deltaX = x - gesture.accelX;
    
    if (abs(deltaX) > 30) {
        if (deltaX > 0) {
            // Right swipe - increase brightness
            ui.brightness = min(255, ui.brightness + 25);
        } else {
            // Left swipe - switch clock face
            ui.currentFace = (ClockFace)((ui.currentFace + 1) % CUSTOM_FACE_COUNT);
            switchClockFace(ui.currentFace);
        }
        gesture.lastGestureTime = now;
    }
}

void handleAutoBrightness() {
    static unsigned long lastBrightnessChange = 0;
    
    // Auto-dim after 30 seconds of inactivity
    if (millis() - ui.lastInteraction > 30000 && ui.brightness > 30) {
        ui.brightness = max(30, ui.brightness - 10);
        watch.setBrightness(ui.brightness);
        lastBrightnessChange = millis();
    }
    
    // Restore brightness after interaction
    if (millis() - ui.lastInteraction < 5000 && ui.brightness < 80 && millis() - lastBrightnessChange > 1000) {
        ui.brightness = 80;
        watch.setBrightness(ui.brightness);
        lastBrightnessChange = millis();
    }
}

void updateSensorData() {
    // Update step counter
    uint32_t currentSteps = watch.getStepCounter();
    if (currentSteps != ui.lastStepCount) {
        ui.lastStepCount = currentSteps;
        lv_label_set_text_fmt(stepLabel, "👟 %lu steps", currentSteps);
    }
    
    // Update temperature
    float currentTemp = watch.readAccelTemp();
    if (abs(currentTemp - ui.coreTemp) > 0.5) {
        ui.coreTemp = currentTemp;
        lv_label_set_text_fmt(tempLabel, "🌡 %.1f°C", currentTemp);
    }
}

uint8_t calculateBatteryPercent(uint16_t voltage) {
    if (voltage >= 4200) return 100;
    if (voltage <= 3300) return 0;
    return (uint8_t)(((voltage - 3300) * 100) / 900);
}