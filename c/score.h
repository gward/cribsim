#ifndef _SCORE_H
#define _SCORE_H

#include <stdbool.h>

#include "cards.h"

typedef bool (*game_callback_func_t)(void *data, int player, uint points);

bool score_starter_jack(card_t starter,
                        game_callback_func_t callback,
                        void *cb_data);

typedef struct {
    // Each field counts the number of points from that source.
    uint fifteens;
    uint pairs;
    uint runs;
    uint flush;
    uint right_jack;
    uint total;
} score_t;

uint count_15s(hand_t *hand);
uint count_pairs(hand_t *hand);
uint count_runs(hand_t *hand);
uint count_flush(hand_t *hand);
uint count_right_jack(hand_t *hand);

score_t score_hand(hand_t *hand);
void score_log(char *prefix, score_t score);

#endif
