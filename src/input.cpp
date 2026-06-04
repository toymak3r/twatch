#include "input.h"
#include "pins.h"
#include <Wire.h>
#include <TouchDrvFT6X36.hpp>

static TouchDrvFT6X36 touch;
static bool wasDown = false;
static int16_t downX = 0, downY = 0;
static uint32_t lastTapMs = 0;

bool touchInit() {
    Wire1.begin(TOUCH_SDA_PIN, TOUCH_SCL_PIN);
    pinMode(TOUCH_INT_PIN, INPUT);
    return touch.begin(Wire1, FT6X36_SLAVE_ADDRESS, TOUCH_SDA_PIN, TOUCH_SCL_PIN);
}

Gesture pollGesture() {
    int16_t x[1], y[1];
    bool down = touch.getPoint(x, y, 1) > 0;
    if (down && !wasDown) { wasDown = true; downX = x[0]; downY = y[0]; return GESTURE_NONE; }
    if (!down && wasDown) {                       // release: decide swipe vs tap
        wasDown = false;
        int dx = (int)x[0] - downX, dy = (int)y[0] - downY;  // last known point
        Gesture g = classifySwipe(dx, dy);
        if (g != GESTURE_NONE) return g;
        uint32_t now = millis();                  // tap
        if (isDoubleTap(lastTapMs, now)) { lastTapMs = 0; return GESTURE_DOUBLE_TAP; }
        lastTapMs = now;
    }
    return GESTURE_NONE;
}
