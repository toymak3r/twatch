#include <unity.h>
#include "../../src/battery.h"

void setUp(void) {}
void tearDown(void) {}

void test_full_above_max(void)  { TEST_ASSERT_EQUAL_UINT8(100, batteryPercent(4250)); }
void test_empty_below_min(void) { TEST_ASSERT_EQUAL_UINT8(0,   batteryPercent(3200)); }
void test_midpoint(void)        { TEST_ASSERT_EQUAL_UINT8(50,  batteryPercent(3750)); }
void test_quarter(void)         { TEST_ASSERT_EQUAL_UINT8(25,  batteryPercent(3525)); }

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_full_above_max);
    RUN_TEST(test_empty_below_min);
    RUN_TEST(test_midpoint);
    RUN_TEST(test_quarter);
    return UNITY_END();
}
