#include <stdlib.h>

#include <check.h>

#include "../cards.h"
#include "testutils.h"

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
END_TEST

START_TEST(test_card_cmp) {
    card_t card1 = {rank : RANK_3, suit : SUIT_CLUB};
    card_t card2 = {rank : RANK_3, suit : SUIT_HEART};
    card_t card3 = {rank : RANK_4, suit : SUIT_HEART};

    ck_assert_int_lt(card_cmp(&card1, &card2), 0);
    ck_assert_int_gt(card_cmp(&card2, &card1), 0);

    ck_assert_int_lt(card_cmp(&card2, &card3), 0);
    ck_assert_int_gt(card_cmp(&card3, &card2), 0);

    card1.suit = SUIT_HEART;
    ck_assert_int_eq(card_cmp(&card1, &card2), 0);
    ck_assert_int_eq(card_cmp(&card2, &card1), 0);
}
END_TEST

START_TEST(test_hand_delete) {
    hand_t *hand = new_hand(4);

    parse_hand(hand, "J♥");
    hand_delete(hand, 0);
    ck_assert_int_eq(hand->ncards, 0);
    ck_assert_int_eq(hand->cards[0].rank, RANK_JOKER);
    ck_assert_int_eq(hand->cards[0].suit, SUIT_NONE);

    parse_hand(hand, "J♥ 5♠ 2♣ Q♥");
    hand_delete(hand, 0);
    ck_assert_int_eq(hand->ncards, 3);
    ck_assert_int_eq(hand->cards[0].rank, RANK_5);
    ck_assert_int_eq(hand->cards[0].suit, SUIT_SPADE);

    parse_hand(hand, "J♥ 5♠ 2♣ Q♥");
    hand_delete(hand, 1);
    ck_assert_int_eq(hand->ncards, 3);
    ck_assert_int_eq(hand->cards[1].rank, RANK_2);
    ck_assert_int_eq(hand->cards[1].suit, SUIT_CLUB);

    parse_hand(hand, "J♥ 5♠ 2♣ Q♥");
    hand_delete(hand, 3);
    ck_assert_int_eq(hand->ncards, 3);
    ck_assert_int_eq(hand->cards[2].rank, RANK_2);
    ck_assert_int_eq(hand->cards[2].suit, SUIT_CLUB);
    ck_assert_int_eq(hand->cards[3].rank, RANK_JOKER);
    ck_assert_int_eq(hand->cards[3].suit, SUIT_NONE);
}
END_TEST

START_TEST(test_hand_str) {
    char buf[20];             // room for four cards

    hand_t *hand = new_hand(6);
    ck_assert_int_eq(hand->ncards, 0);
    ck_assert_str_eq(hand_str(buf, 20, hand), "");

    hand_append(hand, (card_t) {suit : SUIT_CLUB, rank : RANK_5});
    ck_assert_int_eq(hand->ncards, 1);
    ck_assert_str_eq(hand_str(buf, 20, hand), "5♣");

    hand_append(hand, (card_t) {suit : SUIT_DIAMOND, rank : RANK_4});
    ck_assert_int_eq(hand->ncards, 2);
    ck_assert_str_eq(hand_str(buf, 20, hand), "5♣ 4♦");

    hand_append(hand, (card_t) {suit : SUIT_DIAMOND, rank : RANK_4});
    hand_append(hand, (card_t) {suit : SUIT_DIAMOND, rank : RANK_4});
    ck_assert_int_eq(hand->ncards, 4);
    ck_assert_str_eq(hand_str(buf, 20, hand), "5♣ 4♦ 4♦ 4♦");

    // test truncation when converting into a too-small buffer
    ck_assert_str_eq(hand_str(buf, 17, hand), "5♣ 4♦ 4♦ ");

    free(hand);
}
END_TEST

START_TEST(test_new_deck) {
    deck_t *deck = new_deck();
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

START_TEST(test_shuffle_uniform) {
    // Build a minimal 3-card deck: A♣, A♦, A♥.
    deck_t *deck = malloc(sizeof(deck_t) + 3 * sizeof(card_t));
    deck->ncards = 3;

    // Count occurrences of each of the 6 possible orderings.
    int counts[6] = {0};
    int N = 100000;

    for (int i = 0; i < N; i++) {
        deck->cards[0] = (card_t) {.suit = SUIT_CLUB, .rank = RANK_ACE};
        deck->cards[1] = (card_t) {.suit = SUIT_DIAMOND, .rank = RANK_ACE};
        deck->cards[2] = (card_t) {.suit = SUIT_HEART, .rank = RANK_ACE};

        shuffle_deck(deck);

        // Encode the resulting permutation as an integer 0-5 using the
        // Lehmer code.  s0 identifies the first card (0=♣, 1=♦, 2=♥) and
        // contributes 0, 2, or 4.  s1 is compressed to 0 or 1 by removing
        // s0 from the remaining choices, contributing the final bit.
        int s0 = deck->cards[0].suit - SUIT_CLUB;   // 0, 1, or 2
        int s1 = deck->cards[1].suit - SUIT_CLUB;   // 0, 1, or 2
        if (s1 > s0)
            s1--;                           // compress to 0 or 1
        counts[s0 * 2 + s1]++;
    }

    // Chi-square goodness-of-fit test against a uniform distribution over
    // 6 outcomes (5 degrees of freedom).
    //
    // A correct Fisher-Yates shuffle has E[chi-sq] = 5 (= df).
    // The buggy version (j drawn from full range rather than [i, n-1])
    // produces a distribution of 4/27, 4/27, 4/27, 5/27, 5/27, 5/27, giving
    // an expected chi-sq of ~1235 with N=100000 -- easily detectable.
    //
    // Critical value for df=5 at alpha=1e-6 is ~36.2; a correct shuffle
    // will never come close.
    double expected = N / 6.0;
    double chi_sq = 0.0;
    for (int i = 0; i < 6; i++) {
        double diff = counts[i] - expected;
        chi_sq += diff * diff / expected;
    }
    ck_assert_msg(chi_sq < 36.2,
                  "shuffle is not uniform: chi-square=%.2f >= 36.2 (df=5, alpha=1e-6)",
                  chi_sq);

    free(deck);
}
END_TEST

void add_cards_tests(Suite *suite) {
    TCase *tc_cards = tcase_create("cards");
    tcase_add_test(tc_cards, test_card_string);
    tcase_add_test(tc_cards, test_card_cmp);
    tcase_add_test(tc_cards, test_hand_delete);
    tcase_add_test(tc_cards, test_hand_str);
    tcase_add_test(tc_cards, test_new_deck);
    tcase_add_test(tc_cards, test_shuffle_uniform);
    suite_add_tcase(suite, tc_cards);
}
