#include <unity.h>
#include "../../src/gesture.h"

void setUp(void) {}
void tearDown(void) {}

void test_swipe_up(void)    { TEST_ASSERT_EQUAL_INT(GESTURE_SWIPE_UP,   classifySwipe(5, -60)); }
void test_swipe_down(void)  { TEST_ASSERT_EQUAL_INT(GESTURE_SWIPE_DOWN, classifySwipe(-8, 70)); }
void test_too_small(void)   { TEST_ASSERT_EQUAL_INT(GESTURE_NONE,       classifySwipe(5, 10)); }
void test_horizontal(void)  { TEST_ASSERT_EQUAL_INT(GESTURE_NONE,       classifySwipe(80, 5)); }
void test_double_tap_yes(void){ TEST_ASSERT_TRUE(isDoubleTap(1000, 1250)); }
void test_double_tap_no(void) { TEST_ASSERT_FALSE(isDoubleTap(1000, 1600)); }

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_swipe_up);
    RUN_TEST(test_swipe_down);
    RUN_TEST(test_too_small);
    RUN_TEST(test_horizontal);
    RUN_TEST(test_double_tap_yes);
    RUN_TEST(test_double_tap_no);
    return UNITY_END();
}
