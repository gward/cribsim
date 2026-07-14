#include <stdlib.h>

#include <check.h>

#include "../score.h"
#include "./testutils.h"

/* test case: score */

START_TEST(test_count_15s) {
    hand_t *hand = new_hand(5);

    // One card cannot possibly add up to 15.
    hand->ncards = 1;

    parse_hand(hand, "2♦");
    ck_assert_int_eq(count_15s(hand), 0);
    parse_hand(hand, "K♦");
    ck_assert_int_eq(count_15s(hand), 0);

    // Use 4 cards for the next several tests.
    hand->ncards = 4;

    // This takes all 4 cards to get to a single 15.
    parse_hand(hand, "A♥ 3♥ 5♠ 6♦");
    ck_assert_int_eq(count_15s(hand), 1);

    // Again a single 15, but this one takes 3 cards: 2 + 5 + 8.
    parse_hand(hand, "2♦ 3♥ 5♠ 8♥");
    ck_assert_int_eq(count_15s(hand), 1);

    // Two cards make exactly one 15.
    parse_hand(hand, "2♦ 3♥ 7♠ 8♥");
    ck_assert_int_eq(count_15s(hand), 1);

    // This one has two 15s: 2 + 3 + Q, 5 + Q.
    parse_hand(hand, "2♦ 3♥ 5♠ Q♥");
    ck_assert_int_eq(count_15s(hand), 2);

    free(hand);
}
END_TEST

START_TEST(test_count_pairs) {
    hand_t *hand = new_hand(5);

    // No cards: zero, and must not read out of bounds.
    parse_hand(hand, "");
    ck_assert_int_eq(count_pairs(hand), 0);

    // One card: zero, no matter the card.
    hand->ncards = 1;
    parse_hand(hand, "2♦");
    ck_assert_int_eq(count_pairs(hand), 0);

    // Two cards might score, but not these two.
    hand->ncards = 2;
    parse_hand(hand, "2♦ 3♥");
    ck_assert_int_eq(count_pairs(hand), 0);

    // But make them a pair, and now we're talking.
    parse_hand(hand, "2♦ 2♥");
    ck_assert_int_eq(count_pairs(hand), 1);

    // Test pairs in a bigger hand.
    hand->ncards = 4;
    parse_hand(hand, "2♦ 2♥ 5♣ 5♠");
    ck_assert_int_eq(count_pairs(hand), 2);

    // 3 of a kind is 3 pairs.
    parse_hand(hand, "2♦ 2♥ 2♠ 5♠");
    ck_assert_int_eq(count_pairs(hand), 3);

    free(hand);
}
END_TEST

START_TEST(test_count_runs) {
    hand_t *hand = new_hand(6);

    parse_hand(hand, "4♥ 5♦ 5♠ 0♥ J♥");      // nice hand, but no runs
    ck_assert_int_eq(count_runs(hand), 0);

    parse_hand(hand, "4♥ 5♦ 6♠ 0♥ J♥");      // single run of 3
    ck_assert_int_eq(count_runs(hand), 3);

    parse_hand(hand, "4♥ 5♦ 9♠ 0♥ J♥");      // single run of 3, but at the end
    ck_assert_int_eq(count_runs(hand), 3);

    parse_hand(hand, "4♥ 8♦ 9♠ 0♥ J♥");      // single run of 4
    ck_assert_int_eq(count_runs(hand), 4);

    parse_hand(hand, "4♥ 8♦ 8♠ 9♠ 0♥ Q♥");   // double run of 3: 6 points
    ck_assert_int_eq(count_runs(hand), 6);

    parse_hand(hand, "4♥ 8♦ 8♠ 9♠ 0♥ 0♥");   // double double run of 3: 12 points
    ck_assert_int_eq(count_runs(hand), 12);

    free(hand);
}
END_TEST

START_TEST(test_count_flush) {
    hand_t *hand = new_hand(5);

    /* hand with no starter either is a flush ... */
    hand->ncards = 4;
    parse_hand(hand, "4♥ 6♥ 7♥ K♥");
    ck_assert_int_eq(count_flush(hand), 4);

    /* ... or is not a flush */
    parse_hand(hand, "4♥ 6♥ 7♠ K♥");
    ck_assert_int_eq(count_flush(hand), 0);

    /* but if we add the starter, it might be a flush of 4 ... */
    hand->ncards = 5;
    hand->starter = 4;
    parse_hand(hand, "4♥ 6♥ 7♥ K♥ K♠");
    ck_assert_int_eq(count_flush(hand), 4);

    /* ... or a flush of 5 ... */
    parse_hand(hand, "4♥ 6♥ 7♥ Q♥ K♥");
    ck_assert_int_eq(count_flush(hand), 5);

    /* ... or not a flush at all */
    parse_hand(hand, "4♥ 6♦ 7♥ Q♥ K♥");
    ck_assert_int_eq(count_flush(hand), 0);

    /* but if the starter was the 6♦, it is a flush of 4 after all */
    hand->starter = 1;
    ck_assert_int_eq(count_flush(hand), 4);

    /* edge case: starter is the lowest card */
    hand->starter = 0;
    parse_hand(hand, "4♦ 6♥ 7♥ Q♥ K♥");
    ck_assert_int_eq(count_flush(hand), 4);

    free(hand);
}

START_TEST(test_count_right_jack) {
    hand_t *hand = new_hand(5);

    parse_hand(hand, "4♥ 7♣ 9♣ J♦ J♥");

    // starter card not set: no points possible
    ck_assert_int_eq(count_right_jack(hand), 0);

    // starter card does not match either jack: no points
    hand->starter = 1;
    ck_assert_int_eq(count_right_jack(hand), 0);

    // starter card matches a jack: one point
    hand->starter = 0;
    ck_assert_int_eq(count_right_jack(hand), 1);

    // starter card is a jack: still one point (it'll be two points to
    // the dealer because the starter will be both in their hand and
    // the crib)
    hand->starter = 3;
    ck_assert_int_eq(count_right_jack(hand), 1);

    free(hand);
}

START_TEST(test_score_hand) {
    hand_t *hand = new_hand(5);

    hand->ncards = 4;
    parse_hand(hand, "5♦ 5♠ 0♥ J♥");         // four 15s and a pair
    ck_assert_int_eq(score_hand(hand).total, 10);

    hand->ncards = 5;
    parse_hand(hand, "6♦ 7♥ 7♠ 8♠ 9♥");     // three 15s and a double run of 4
    ck_assert_int_eq(score_hand(hand).total, 16);

    hand->ncards = 4;
    parse_hand(hand, "6♠ 7♠ 8♠ 9♠");        // two 15s, run of 4, flush of 4
    ck_assert_int_eq(score_hand(hand).total, 12);

    parse_hand(hand, "6♠ 7♠ 8♠ 9♠ J♠");     // two 15s, run of 4, flush of 5, right jack
    hand->starter = 2;
    ck_assert_int_eq(score_hand(hand).total, 14);

    // But if the starter card was a jack, that's not counted here.
    hand->starter = 4;
    ck_assert_int_eq(score_hand(hand).total, 13);

    free(hand);
}
END_TEST

void add_score_tests(Suite *suite) {
    TCase *tc_score = tcase_create("score");

    tcase_add_test(tc_score, test_count_15s);
    tcase_add_test(tc_score, test_count_pairs);
    tcase_add_test(tc_score, test_count_runs);
    tcase_add_test(tc_score, test_count_flush);
    tcase_add_test(tc_score, test_count_right_jack);
    tcase_add_test(tc_score, test_score_hand);
    suite_add_tcase(suite, tc_score);
}
