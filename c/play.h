#ifndef _PLAY_H
#define _PLAY_H

#include "cards.h"
#include "score.h"

typedef struct {
    // Total points for each player:
    //   - player 0 is nondealer (first deal, starts pegging, counts first)
    //   - player 1 is dealer
    //
    // Note that the identity of player 0 and 1 will swap on each hand!
    uint scores[2];

    // -1 if no winner yet, otherwise 0 or 1 for the player who just hit 121
    int winner;
} game_state_t;

game_state_t game_state_init();

void add_starter(hand_t *hand, card_t starter);
void play_hand(game_state_t *game_state, deck_t *deck);

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

peg_state_t *new_peg_state(int ncards);
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

bool evaluate_hands(game_state_t *game_state,
                    int nplayers,
                    hand_t *hands[],
                    hand_t *crib,
                    card_t starter);
bool peg_hands(int nplayers,
               peg_state_t *peg,
               hand_t *hands[],
               peg_func_t select[],
               game_callback_func_t callback,
               void *cb_data);

#endif
