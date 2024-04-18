#include <stdlib.h>

#include <check.h>

#include "../play.h"

/* test case: cards */

START_TEST(test_card_string) {
    card_t card = {0, 0};
    char buf[5];
    ck_assert_str_eq(card_debug(buf, card), "00:0");
    ck_assert_str_eq(card_str(buf, card), "*");

    card.suit = SUIT_CLUB;
    card.rank = RANK_ACE;
    ck_assert_str_eq(card_debug(buf, card), "01:1");
    ck_assert_str_eq(card_str(buf, card), "A♣");

    card.suit = SUIT_HEART;
    ck_assert_str_eq(card_debug(buf, card), "01:3");
    ck_assert_str_eq(card_str(buf, card), "A♥");

    card.rank = RANK_2;
    ck_assert_str_eq(card_debug(buf, card), "02:3");
    ck_assert_str_eq(card_str(buf, card), "2♥");

    card.rank = RANK_KING;
    ck_assert_str_eq(card_debug(buf, card), "13:3");
    ck_assert_str_eq(card_str(buf, card), "K♥");
}

START_TEST(test_new_deck) {
    deck_t* deck = new_deck();
    ck_assert_int_eq(deck->ncards, 52);
    ck_assert_int_eq(deck->cards[0].rank, RANK_ACE);
    ck_assert_int_eq(deck->cards[0].suit, SUIT_CLUB);
    ck_assert_int_eq(deck->cards[1].rank, RANK_ACE);
    ck_assert_int_eq(deck->cards[1].suit, SUIT_DIAMOND);
    ck_assert_int_eq(deck->cards[2].rank, RANK_ACE);
    ck_assert_int_eq(deck->cards[2].suit, SUIT_HEART);
    ck_assert_int_eq(deck->cards[3].rank, RANK_ACE);
    ck_assert_int_eq(deck->cards[3].suit, SUIT_SPADE);
    ck_assert_int_eq(deck->cards[51].rank, RANK_KING);
    ck_assert_int_eq(deck->cards[51].suit, SUIT_SPADE);
    free(deck);
}

/* test case: play */

START_TEST(test_score_hand) {
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

Suite* cribsum_suite(void) {
    Suite* suite = suite_create("cribsim");
    TCase* tc_cards = tcase_create("cards");
    TCase* tc_play = tcase_create("play");

    tcase_add_test(tc_cards, test_card_string);
    tcase_add_test(tc_cards, test_new_deck);
    suite_add_tcase(suite, tc_cards);

    tcase_add_test(tc_play, test_score_hand);
    suite_add_tcase(suite, tc_play);

    return suite;
}

int main(void) {
    Suite* suite = cribsum_suite();
    SRunner* runner = srunner_create(suite);

    srunner_run_all(runner, CK_NORMAL);
    int num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
