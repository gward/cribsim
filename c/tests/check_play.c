#include <stdlib.h>

#include <check.h>

#include "../play.h"

START_TEST(test_score_hand)
{
    hand_t* hand = new_hand(5);

    // Empty hand: zero score.
    hand->ncards = 0;
    ck_assert_int_eq(score_hand(hand), 0);

    free(hand);
}
END_TEST

Suite* play_suite(void) {
    Suite* suite;
    TCase* tc_core;

    suite = suite_create("play");
    tc_core = tcase_create("core");
    tcase_add_test(tc_core, test_score_hand);
    suite_add_tcase(suite, tc_core);

    return suite;
}

int main(void) {
    int num_failed;
    Suite* suite;
    SRunner* runner;

    suite = play_suite();
    runner = srunner_create(suite);

    srunner_run_all(runner, CK_NORMAL);
    num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
