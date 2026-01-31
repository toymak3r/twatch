/**
 * @file      DisplayBasics.ino
 * @author    T-Watch S3 Display & Touch Basics
 * @license   MIT
 * @date      2026-01-31
 * 
 * ESSENTIAL EXAMPLE - Display and Touch Fundamentals
 * Learn: Touch interaction, basic graphics, UI elements
 */

#include <LilyGoLib.h>
#include <LV_Helper.h>

// Touch and display globals
lv_obj_t *titleLabel;
lv_obj_t *touchLabel;
lv_obj_t *infoLabel;
uint16_t touchX = 0, touchY = 0;
bool isPressed = false;

void setup() {
    Serial.begin(115200);
    Serial.println("=== T-Watch Display & Touch Basics ===");

    watch.begin();
    beginLvglHelper();
    
    createBasicUI();
    
    Serial.println("✅ Display and touch initialized");
    Serial.println("👆 Touch the screen to see coordinates");
}

void loop() {
    handleTouchInput();
    updateDisplayInfo();
    
    lv_task_handler();
    delay(5); // 50Hz refresh rate
}

void createBasicUI() {
    // Dark background
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    
    // Title
    titleLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(titleLabel, "📱 T-Watch Display Basics");
    lv_obj_set_style_text_color(titleLabel, lv_color_hex(0x00ff00), 0);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 10);
    
    // Touch info
    touchLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(touchLabel, "Touch: -- , --");
    lv_obj_set_style_text_color(touchLabel, lv_color_hex(0xffff00), 0);
    lv_obj_align(touchLabel, LV_ALIGN_CENTER, 0, 0);
    
    // System info
    infoLabel = lv_label_create(lv_scr_act());
    lv_label_set_text(infoLabel, "Press and drag to draw!");
    lv_obj_set_style_text_color(infoLabel, lv_color_hex(0xaaaaaa), 0);
    lv_obj_align(infoLabel, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    // Set medium brightness for visibility
    watch.setBrightness(120);
}

void handleTouchInput() {
    bool currentlyPressed = watch.getTouched();
    
    if (currentlyPressed && !isPressed) {
        // Touch started
        isPressed = true;
        Serial.println("👆 Touch detected");
    } else if (!currentlyPressed && isPressed) {
        // Touch released
        isPressed = false;
        Serial.println("👋 Touch released");
        
        // Create a small animation at touch point
        createTouchAnimation(touchX, touchY);
    }
    
    // Get touch coordinates if pressed
    if (currentlyPressed) {
        watch.getPoint(&touchX, &touchY);
        
        // Simple visual feedback - change screen color slightly
        uint8_t brightness = map(touchX, 0, 240, 50, 200);
        watch.setBrightness(brightness);
    }
}

void updateDisplayInfo() {
    if (isPressed) {
        lv_label_set_text_fmt(touchLabel, "Touch: %d, %d", touchX, touchY);
        
        // Change label color based on touch position
        lv_color_t touchColor = lv_color_hex((touchX << 8) | (255 - touchY));
        lv_obj_set_style_text_color(touchLabel, touchColor, 0);
    } else {
        lv_label_set_text(touchLabel, "Touch: -- , --");
        lv_obj_set_style_text_color(touchLabel, lv_color_hex(0xffff00), 0);
    }
}

void createTouchAnimation(int16_t x, int16_t y) {
    // Create temporary circle at touch point
    lv_obj_t *circle = lv_obj_create(lv_scr_act());
    lv_obj_set_size(circle, 30, 30);
    lv_obj_set_pos(circle, x - 15, y - 15);
    lv_obj_set_style_bg_color(circle, lv_color_hex(0x00ff00), 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_50, 0);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    
    // Simple fade out effect by shrinking
    for (int i = 30; i > 0; i -= 3) {
        lv_obj_set_size(circle, i, i);
        lv_obj_set_pos(circle, x - i/2, y - i/2);
        lv_obj_set_style_bg_opa(circle, (i * 255) / 30, 0);
        lv_task_handler();
        delay(20);
    }
    
    // Remove the circle
    lv_obj_del(circle);
}

// Utility function to map values (like Arduino's map)
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}