#include <unity.h>
#include "../../src/sync_policy.h"

void setUp(void) {}
void tearDown(void) {}

void test_never_synced(void)      { TEST_ASSERT_TRUE(needsNtpSync(false, 0, 155)); }
void test_same_day_no_sync(void)  { TEST_ASSERT_FALSE(needsNtpSync(true, 155, 155)); }
void test_new_day_sync(void)      { TEST_ASSERT_TRUE(needsNtpSync(true, 155, 156)); }

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_never_synced);
    RUN_TEST(test_same_day_no_sync);
    RUN_TEST(test_new_day_sync);
    return UNITY_END();
}
