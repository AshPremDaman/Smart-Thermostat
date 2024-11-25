#include <unity.h>

#include "CycleList.hpp"

// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

void test_linked_list_insert(void) {
    CycleList cycle_list = CycleList();
    TEST_ASSERT_EQUAL(0, cycle_list.size());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_linked_list_insert);
    UNITY_END();

    return 0;
}