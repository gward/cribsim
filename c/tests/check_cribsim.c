#include <stdlib.h>

#include <check.h>

#include "../play.h"

START_TEST(test_score_hand)
{
    hand_t* hand = new_hand(5);

    // Empty hand: zero score.
    hand->ncards = 0;
    ck_assert_int_eq(score_hand(hand), 0);

    // One card: still zero, no matter the card.
    hand->ncards = 1;
    ck_assert_int_eq(score_hand(hand), 0);

    free(hand);
}
END_TEST

Suite* play_suite(void) {
    Suite* suite = suite_create("cribsim");
    TCase* tc_play = tcase_create("play");

    tcase_add_test(tc_play, test_score_hand);
    suite_add_tcase(suite, tc_play);

    return suite;
}

int main(void) {
    Suite* suite = play_suite();
    SRunner* runner = srunner_create(suite);

    srunner_run_all(runner, CK_NORMAL);
    int num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
