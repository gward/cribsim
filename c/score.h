#ifndef _SCORE_H
#define _SCORE_H

#include "cards.h"

uint count_15s(hand_t *hand);
uint count_pairs(hand_t *hand);
uint count_runs(hand_t *hand);
uint score_hand(hand_t *hand);

#endif
