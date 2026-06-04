#pragma once
#include <stdint.h>
#include <stdlib.h>
enum Gesture { GESTURE_NONE, GESTURE_SWIPE_UP, GESTURE_SWIPE_DOWN, GESTURE_DOUBLE_TAP };
#define SWIPE_MIN_PX        40
#define DOUBLE_TAP_MAX_MS   400
// Vertical swipe only: dominant vertical movement over threshold.
inline Gesture classifySwipe(int dx, int dy) {
    if (abs(dy) > abs(dx) && abs(dy) >= SWIPE_MIN_PX)
        return dy < 0 ? GESTURE_SWIPE_UP : GESTURE_SWIPE_DOWN;
    return GESTURE_NONE;
}
inline bool isDoubleTap(uint32_t prevTapMs, uint32_t nowMs) {
    return (nowMs - prevTapMs) <= DOUBLE_TAP_MAX_MS;
}
