#include <stdio.h>
#include <stdlib.h>

#include <check.h>

#include "../cards.h"
#include "../score.h"
#include "../stringbuilder.h"
#include "../play.h"

/* Parse a string like "A♥ 3♥ 5♠ 6♦" into cards, and use it to populate hand. */
static void parse_hand(hand_t *dest, char cards[]) {
    hand_truncate(dest);
    char *next = cards + 0;
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

        char *suit_str = next;

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

/* test case: stringbuilder */

static void assert_stringbuilder(stringbuilder_t *sb, char *expect_str, size_t expect_cap) {
    size_t expect_len = strlen(expect_str);
    ck_assert_str_eq(sb_as_string(sb), expect_str);
    ck_assert_int_eq(strlen(sb->mem), expect_len);
    ck_assert_int_eq((int) sb->mem[0], (int) expect_str[0]);
    ck_assert_int_eq((int) sb->mem[expect_len - 1], (int) expect_str[expect_len - 1]);
    ck_assert_int_eq((int) sb->mem[expect_len], 0);
    ck_assert_int_eq(sb->len, expect_len);
    ck_assert_int_eq(sb->cap, expect_cap);
}

START_TEST(test_stringbuilder_basics) {
    stringbuilder_t sb;
    sb_init(&sb, 1);

    ck_assert_int_eq(sb.len, 0);
    ck_assert_int_eq(sb.cap, 1);
    ck_assert_str_eq(sb_as_string(&sb), "");

    // A 31-byte string fits comfortable, as long as we double the capacity
    // (buffer size) several times.
    sb_append(&sb, "this is a moderately long strin");
    assert_stringbuilder(&sb, "this is a moderately long strin", 32);

    // Add one more byte and we have to double the capacity again.
    sb_append_char(&sb, 'g');
    assert_stringbuilder(&sb, "this is a moderately long string", 64);

    sb_close(&sb);
    sb_init(&sb, 1);

    // Same thing, but as a single string append.
    sb_append(&sb, "this is a moderately long string");
    assert_stringbuilder(&sb, "this is a moderately long string", 64);
}
END_TEST

START_TEST(test_stringbuilder_printf) {
    stringbuilder_t sb;
    sb_init(&sb, 8);

    sb_printf(&sb,
              "hello %s, my name is %s and I am %.1f years old",
              "world",
              "bob",
              37.59);
    assert_stringbuilder(&sb,
                         "hello world, my name is bob and I am 37.6 years old",
                         64);
}

START_TEST(test_stringbuilder_append_int) {
    stringbuilder_t sb;
    sb_init(&sb, 20);

    // Exercise sb_append_int() without having to expand the buffer.
    sb_append_int(&sb, 234254);
    sb_append_char(&sb, ',');
    sb_append_int(&sb, -549202);
    assert_stringbuilder(&sb, "234254,-549202", 20);

    // Same, but this time the buffer has to grow.
    sb_close(&sb);
    sb_init(&sb, 1);
    sb_append_int(&sb, -234254);
    assert_stringbuilder(&sb, "-234254", 8);
    sb_append(&sb, " , ");
    assert_stringbuilder(&sb, "-234254 , ", 16);
    sb_append_int(&sb, 938682);
    assert_stringbuilder(&sb, "-234254 , 938682", 32);
}
END_TEST

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

    hand_append(hand, (card_t) {suit: SUIT_CLUB, rank: RANK_5});
    ck_assert_int_eq(hand->ncards, 1);
    ck_assert_str_eq(hand_str(buf, 20, hand), "5♣");

    hand_append(hand, (card_t) {suit: SUIT_DIAMOND, rank: RANK_4});
    ck_assert_int_eq(hand->ncards, 2);
    ck_assert_str_eq(hand_str(buf, 20, hand), "5♣ 4♦");

    hand_append(hand, (card_t) {suit: SUIT_DIAMOND, rank: RANK_4});
    hand_append(hand, (card_t) {suit: SUIT_DIAMOND, rank: RANK_4});
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

/* test case: play */

typedef struct {
    char *hand_0;
    char *hand_1;
    int expect_counts[MAX_ROUNDS];
    uint expect_points[2];
    char *expect_plays[MAX_ROUNDS];
} peg_test_t;

static peg_test_t peg_tests[] = {
    // These two hands will result in no 15s, no pairs, no runs, no 31, and
    // no starting over. Each player alternates playing their lowest card --
    // 2, A, 2, A, 4, A, 6, 8 -- until both are out. Dealer (player 1) gets
    // 1 point for the go.
    {
        hand_0: "2♥ 2♥ 4♥ 6♥",
        hand_1: "A♠ A♣ A♥ 8♠",
        expect_plays: {"2♥ A♠ 2♥ A♣ 4♥ A♥ 6♥ 8♠", NULL, NULL},
        expect_counts: {25, -1, -1},
        expect_points: {0, 1},
    },

    // Play A, 4, 3, 4, 3: 15 for player 0; then 6, 4, 6: 31 for player 1.
    {
        hand_0: "A♥ 3♥ 3♦ 4♥",
        hand_1: "4♠ 4♣ 6♥ 6♠",
        expect_plays: {"A♥ 4♠ 3♥ 4♣ 3♦ 6♥ 4♥ 6♠", NULL, NULL},
        expect_counts: {31, -1, -1},
        expect_points: {2, 2},
    },

    // Play 7, 4, 8, 5: count is 24 and player 0 is blocked.
    // Play 7: count is 31, 2 points to player 1, reset count to zero.
    // Play 10, 9, 10: count is 29, both players are out, 1 point to player 0.
    {
        hand_0: "7♦ 8♣ 0♥ 0♣",
        hand_1: "4♦ 5♥ 7♣ 9♣",
        expect_plays: {"7♦ 4♦ 8♣ 5♥ 7♣", "0♥ 9♣ 0♣", NULL},
        expect_counts: {31, 29, -1},
        expect_points: {1, 2},
    },

    // Play 0, K, 0: count is 30, both blocked, 1 point to player 0.
    // Play K, 0, K: count is 30, both blocked, 1 point to player 1.
    // Play 0, K: count is 20, both players are out, 1 point to player 1.
    {
        hand_0: "0♣ 0♦ 0♥ 0♠",
        hand_1: "K♣ K♦ K♥ K♠",
        expect_plays: {"0♣ K♣ 0♦", "K♦ 0♥ K♥", "0♠ K♠"},
        expect_counts: {30, 30, 20},
        expect_points: {1, 2},
    },

    // Play 2, 4, 2, 5, 2: count is 15, 2 points to player 0.
    // Play 6, 9: count is 30, 1 point to player 1 for the go.
    // Play 7: count is 7, both players out, 1 point to player 0.
    {
        hand_0: "2♣ 2♦ 2♥ 9♠",
        hand_1: "4♥ 5♦ 6♦ 7♥",
        expect_plays: {"2♣ 4♥ 2♦ 5♦ 2♥ 6♦ 9♠", "7♥", NULL},
        expect_counts: {30, 7, -1},
        expect_points: {3, 1},
    },

    // Play 3, A, 5, A, 5: count is 15, 2 points to player 0.
    // Play 8: count is 23, player 0 blocked, 1 point to player 1.
    // Play K, 9: count is 19, both players out, 1 point to player 1.
    {
        hand_0: "3♣ 5♦ 5♥ K♣",
        hand_1: "A♦ A♠ 8♦ 9♥",
        expect_plays: {"3♣ A♦ 5♦ A♠ 5♥ 8♦", "K♣ 9♥", NULL},
        expect_counts: {23, 19, -1},
        expect_points: {2, 2},
    },

    // Play J, A, J, 5: count is 26, both players blocked, 1 point to player 1.
    // Play K, 7, K: count is 27, player 0 out, player 1 blocked: 1 point to player 0.
    // Play 9: count is 9, 1 point to player 1 for last card.
    {
        hand_0: "J♦ J♥ K♣ K♥",
        hand_1: "A♣ 5♦ 7♦ 9♦",
        expect_plays: {"J♦ A♣ J♥ 5♦", "K♣ 7♦ K♥", "9♦"},
        expect_counts: {26, 27, 9},
        expect_points: {1, 2},
    },

    // This one has a 15, then a pair and a 31 -- all for player 0.
    // Play 6, 2, 7: count is 15, 2 points to player 0.
    // Play 8, 8: count is 31, pair for player 0 (4 more points).
    // Play J, 8, Q: count is 28, last card for player 1 (1 point).
    {
        hand_0: "6♠ 7♣ 8♥ 8♠",
        hand_1: "2♠ 8♦ J♣ Q♦",
        expect_plays: {"6♠ 2♠ 7♣ 8♦ 8♥", "J♣ 8♠ Q♦", NULL},
        expect_counts: {31, 28, -1},
        expect_points: {6, 1},
    },

    // 3-of-a-kind and a run of 3: very exciting.
    // Play 3, 5, 5: count is 13, 2 points to player 0 for pair.
    // Play 5: count is 18, 6 points to player 1 for triple.
    // Play 6: count is 24, player 1 blocked.
    // Play 7: count is 31, 2 points plus 3 points for run of 3 to player 0.
    // Play J, Q: count is 20, 1 point to player 1 for last card.
    {
        hand_0: "3♠ 5♣ 6♥ 7♠",
        hand_1: "5♦ 5♠ J♣ Q♦",
        expect_plays: {"3♠ 5♦ 5♣ 5♠ 6♥ 7♠", "J♣ Q♦", NULL},
        expect_counts: {31, 20, -1},
        expect_points: {7, 7},
    },

    // Play 6, 2, 7: count is 15, 2 points to player 0.
    // Play 6, 8: count is 29, player 1 blocked, 3 to player 0 for run, plus 1 for go.
    // Play 7, 9, K: count is 26, 1 point to player 1 for last card.
    {
        hand_0: "6♦ 7♠ 8♥ 9♥",
        hand_1: "2♣ 6♠ 7♥ K♣",
        expect_plays: {"6♦ 2♣ 7♠ 6♠ 8♥", "7♥ 9♥ K♣", NULL},
        expect_counts: {29, 26, -1},
        expect_points: {6, 1},
    }
};

bool count_pegging(void *data, int player, uint points) {
    game_state_t *game_state = data;
    game_state->scores[player] += points;
    return false;
}

START_TEST(test_peg_hands) {
    hand_t *hands[2] = {new_hand(4), new_hand(4)};
    peg_state_t *peg = new_peg_state(4);
    peg_func_t select_func[2] = {peg_select_low, peg_select_low};

    // This test is purely about counting points during pegging, not about
    // strategy. Deliberately using a naive and predictable strategy,
    // peg_select_low(), to concentrate on just the pegging points.

    int i = _i;
    peg_test_t tc = peg_tests[i];

    printf("peg_tests[%d]: expect_counts={%d, %d, %d}, expect_points={%d, %d}\n",
           i,
           tc.expect_counts[0],
           tc.expect_counts[1],
           tc.expect_counts[2],
           tc.expect_points[0],
           tc.expect_points[1]);

    game_state_t game_state = game_state_init();
    parse_hand(hands[0], tc.hand_0);
    parse_hand(hands[1], tc.hand_1);
    peg_hands(2, peg, hands, select_func, count_pegging, &game_state);

    printf("peg_tests[%d]: actual_counts={%d, %d, %d}, actual_points={%d, %d}\n",
           i,
           peg->counts[0],
           peg->counts[1],
           peg->counts[2],
           peg->points[0],
           peg->points[1]);

    ck_assert_int_gt(peg->num_rounds, 0);
    ck_assert_int_le(peg->num_rounds, MAX_ROUNDS);
    for (int j = 0; j < MAX_ROUNDS; j++) {
        ck_assert_int_eq(peg->counts[j], tc.expect_counts[j]);
    }
    ck_assert_int_eq(peg->points[0], tc.expect_points[0]);
    ck_assert_int_eq(peg->points[1], tc.expect_points[1]);

    size_t buf_size = 50;
    char buf[buf_size];

    for (i = 0; i < MAX_ROUNDS; i++) {
        hand_t *played = peg->cards_played[i];
        if (tc.expect_plays[i] == NULL) {
            ck_assert_ptr_null(played);
        }
        else {
            ck_assert_ptr_nonnull(played);
            ck_assert_str_eq(tc.expect_plays[i], hand_str(buf, buf_size, played));
        }
    }
    peg_state_free(peg);
}
END_TEST

START_TEST(test_add_starter) {
    hand_t *hand = new_hand(5);
    parse_hand(hand, "4♠ 7♠ 9♠ 0♠");
    add_starter(hand, (card_t) {rank: RANK_5, suit: SUIT_CLUB});

    ck_assert_int_eq(hand->starter, 1);
    ck_assert_int_eq(hand->cards[1].rank, RANK_5);
    ck_assert_int_eq(hand->cards[1].suit, SUIT_CLUB);
}
END_TEST

typedef struct {
    char *hand_0;
    char *hand_1;
    char *crib;
    card_t starter;
    uint initial_scores[2];
    uint expect_scores[2];
    int expect_winner;
    bool expect_done;
} evaluate_hands_test_t;

static evaluate_hands_test_t evaluate_hands_tests[] = {
    // Two simple hands:
    // - player 1 (dealer) gets 1 point pegging for last card
    // - player 0 gets 1 pair = 2 points
    // - player 1 gets 3 pairs = 6 points
    // - crib is totally useless
    {
        hand_0: "2♥ 2♦ 4♥ 6♥",
        hand_1: "A♠ A♣ A♥ 8♠",
        crib: "2♠ 4♣ 6♣ 8♣",
        starter: {rank: RANK_10, suit: SUIT_CLUB},
        initial_scores: {0, 0},
        expect_scores: {2, 7},
        expect_winner: -1,
        expect_done: false,
    },

    // Same, but this time the starter card is a jack: dealer gets 2 additional points.
    {
        hand_0: "2♥ 2♦ 4♥ 6♥",
        hand_1: "A♠ A♣ A♥ 8♠",
        crib: "2♠ 4♣ 6♣ 8♣",
        starter: {rank: RANK_JACK, suit: SUIT_CLUB},
        initial_scores: {0, 0},
        expect_scores: {2, 9},
        expect_winner: -1,
        expect_done: false,
    },

    // Game is nearly over: player 0 has 120, player 1 has 119. Turning up a
    // jack bumps player 1 over the edge and we have a winner. Player 0 doesn't
    // get a chance to count anything: no pegging, no scoring of hands.
    {
        hand_0: "2♥ 2♦ 4♥ 6♥",
        hand_1: "A♠ A♣ A♥ 8♠",
        crib: "2♠ 4♣ 6♣ 8♣",
        starter: {rank: RANK_JACK, suit: SUIT_CLUB},
        initial_scores: {120, 119},
        expect_scores: {120, 121},
        expect_winner: 1,
        expect_done: true,
    },
};

START_TEST(test_evaluate_hands) {
    hand_t *hands[2] = {new_hand(5), new_hand(5)};
    hand_t *crib = new_hand(5);

    evaluate_hands_test_t tc = evaluate_hands_tests[_i];

    parse_hand(hands[0], tc.hand_0);
    parse_hand(hands[1], tc.hand_1);
    parse_hand(crib, tc.crib);

    bool done;
    game_state_t game_state = game_state_init();
    game_state.scores[0] = tc.initial_scores[0];
    game_state.scores[1] = tc.initial_scores[1];

    done = evaluate_hands(&game_state, 2, hands, crib, tc.starter);
    ck_assert_int_eq(game_state.scores[0], tc.expect_scores[0]);
    ck_assert_int_eq(game_state.scores[1], tc.expect_scores[1]);
    ck_assert_int_eq(game_state.winner, tc.expect_winner);
    ck_assert(done == tc.expect_done);

    free(hands[0]);
    free(hands[1]);
    free(crib);
}
END_TEST

Suite *cribsum_suite(void) {
    Suite *suite = suite_create("cribsim");
    TCase *tc_stringbuilder = tcase_create("stringbuilder");
    TCase *tc_cards = tcase_create("cards");
    TCase *tc_score = tcase_create("score");
    TCase *tc_play = tcase_create("play");
    int ntests;

    tcase_add_test(tc_stringbuilder, test_stringbuilder_basics);
    tcase_add_test(tc_stringbuilder, test_stringbuilder_printf);
    tcase_add_test(tc_stringbuilder, test_stringbuilder_append_int);
    suite_add_tcase(suite, tc_stringbuilder);

    tcase_add_test(tc_cards, test_card_string);
    tcase_add_test(tc_cards, test_card_cmp);
    tcase_add_test(tc_cards, test_hand_delete);
    tcase_add_test(tc_cards, test_hand_str);
    tcase_add_test(tc_cards, test_new_deck);
    suite_add_tcase(suite, tc_cards);

    tcase_add_test(tc_score, test_count_15s);
    tcase_add_test(tc_score, test_count_pairs);
    tcase_add_test(tc_score, test_count_runs);
    tcase_add_test(tc_score, test_count_flush);
    tcase_add_test(tc_score, test_count_right_jack);
    tcase_add_test(tc_score, test_score_hand);
    suite_add_tcase(suite, tc_score);

    ntests = sizeof(peg_tests) / sizeof(peg_test_t);
    tcase_add_loop_test(tc_play, test_peg_hands, 0, ntests);

    tcase_add_test(tc_play, test_add_starter);
    ntests = sizeof(evaluate_hands_tests) / sizeof(evaluate_hands_test_t);
    tcase_add_loop_test(tc_play, test_evaluate_hands, 0, ntests);
    suite_add_tcase(suite, tc_play);

    return suite;
}

int main(void) {
    Suite *suite = cribsum_suite();
    SRunner *runner = srunner_create(suite);

    srunner_run_all(runner, CK_NORMAL);
    int num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
