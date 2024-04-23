#include <stdio.h>
#include <stdlib.h>

#include <check.h>

#include "../score.h"
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

START_TEST(test_card_cmp) {
    card_t card1 = {rank: RANK_3, suit: SUIT_CLUB};
    card_t card2 = {rank: RANK_3, suit: SUIT_HEART};
    card_t card3 = {rank: RANK_4, suit: SUIT_HEART};

    ck_assert_int_lt(card_cmp(&card1, &card2), 0);
    ck_assert_int_gt(card_cmp(&card2, &card1), 0);

    ck_assert_int_lt(card_cmp(&card2, &card3), 0);
    ck_assert_int_gt(card_cmp(&card3, &card2), 0);

    card1.suit = SUIT_HEART;
    ck_assert_int_eq(card_cmp(&card1, &card2), 0);
    ck_assert_int_eq(card_cmp(&card2, &card1), 0);
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

/* test case: score */

/* Parse a string like "A♥ 3♥ 5♠ 6♦" into cards, and use it to populate hand. */
static void parse_hand(hand_t* dest, char cards[]) {
    hand_truncate(dest);
    char* next = cards + 0;
    int i = 0;
    while (*next != '\0') {
        for (; *next == ' '; next++) {
            /* consume optional whitespace */
        }

        char rank_ch = *(next++);
        card_t card = {0, 0};

        switch (rank_ch) {
        case '*':
            card.rank = RANK_JOKER;
            break;
        case 'A':
            card.rank = RANK_ACE;
            break;
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            card.rank = ((rank_t) (rank_ch - '2')) + RANK_2;
            break;
        case '0':
            card.rank = RANK_10;
            break;
        case 'J':
            card.rank = RANK_JACK;
            break;
        case 'Q':
            card.rank = RANK_QUEEN;
            break;
        case 'K':
            card.rank = RANK_KING;
            break;
        default:
            fprintf(stderr, "abort: invalid rank char: %c (%02hhx)\n", rank_ch, rank_ch);
            abort();
        }

        char* suit_str = next;

        if (strncmp(suit_str, "♣", 3) == 0) {
            card.suit = SUIT_CLUB;
        }
        else if (strncmp(suit_str, "♦", 3) == 0) {
            card.suit = SUIT_DIAMOND;
        }
        else if (strncmp(suit_str, "♥", 3) == 0) {
            card.suit = SUIT_HEART;
        }
        else if (strncmp(suit_str, "♠", 3) == 0) {
            card.suit = SUIT_SPADE;
        }
        else {
            fprintf(stderr, "abort: invalid suit string: %s\n", suit_str);
            abort();
        }

        next += 3;              /* consume the 3 bytes of a UTF-encoded suit string */

        hand_append(dest, card);
        i++;
    }

}

START_TEST(test_count_15s) {
    hand_t* hand = new_hand(5);

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
    hand_t* hand = new_hand(5);

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
    hand_t* hand = new_hand(6);

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
    hand_t* hand = new_hand(5);

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

START_TEST(test_score_hand) {
    hand_t* hand = new_hand(5);

    hand->ncards = 4;
    parse_hand(hand, "5♦ 5♠ 0♥ J♥");         // four 15s and a pair
    ck_assert_int_eq(score_hand(hand), 10);

    hand->ncards = 5;
    parse_hand(hand, "6♦ 7♥ 7♠ 8♠ 9♥");     // three 15s and a double run of 4
    ck_assert_int_eq(score_hand(hand), 16);

    hand->ncards = 4;
    parse_hand(hand, "6♠ 7♠ 8♠ 9♠");        // two 15s, run of 4, flush of 4
    ck_assert_int_eq(score_hand(hand), 12);

    free(hand);
}
END_TEST

Suite* cribsum_suite(void) {
    Suite* suite = suite_create("cribsim");
    TCase* tc_cards = tcase_create("cards");
    TCase* tc_score = tcase_create("score");

    tcase_add_test(tc_cards, test_card_string);
    tcase_add_test(tc_cards, test_card_cmp);
    tcase_add_test(tc_cards, test_new_deck);
    suite_add_tcase(suite, tc_cards);

    tcase_add_test(tc_score, test_count_15s);
    tcase_add_test(tc_score, test_count_pairs);
    tcase_add_test(tc_score, test_count_runs);
    tcase_add_test(tc_score, test_count_flush);
    tcase_add_test(tc_score, test_score_hand);
    suite_add_tcase(suite, tc_score);

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
