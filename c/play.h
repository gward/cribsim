#ifndef _PLAY_H
#define _PLAY_H

#include "cards.h"

void add_starter(hand_t *hand, card_t starter);
void play_hand(deck_t *deck);

#define MAX_ROUNDS 3

typedef struct {
    // Keep track of the number of times that the pegging count hits or exceeds
    // 31, and the highest count that we reach each time. With 2 players, 4
    // cards per hand, both players having only value 10 cards (10, J, Q, K),
    // then they will top out at 30, 30, 20. Thus the maximum possible number of
    // counts is 3.
    uint num_rounds;
    hand_t *cur_played;                 // list of cards played on current round
    hand_t *cards_played[MAX_ROUNDS];   // list of cards played on each round
    int counts[MAX_ROUNDS];             // count reached on each round

    // The current count, i.e. sum of the cards played since the count was last
    // reset to zero.
    uint cur_count;

    uint points[2];
    hand_t *avail[2];
} peg_state_t;

void peg_state_free(peg_state_t *peg);

// peg_func_t implements a pegging strategy: each call plays a single
// card by the current player. Returns offset into the array of
// available cards for that player, or -1 for "cannot play".
//
// Caller is responsible for all adjustments to pegging state: moving
// card from avail to played, updating count, whatever.
typedef int (*peg_func_t)(peg_state_t * peg, int player, int other);

int peg_select_low(peg_state_t *peg, int player, int other);
int peg_select_high(peg_state_t *peg, int player, int other);

peg_state_t *peg_hands(hand_t *hand_a, hand_t *hand_b, peg_func_t select_a, peg_func_t select_b);

#endif
