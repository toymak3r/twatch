#pragma once
#include "gesture.h"
bool touchInit();
Gesture pollGesture();   // call repeatedly while awake; returns a gesture or GESTURE_NONE
