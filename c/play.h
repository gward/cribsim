#ifndef _PLAY_H
#define _PLAY_H

#include "cards.h"

uint count_15s(hand_t *hand);
uint count_pairs(hand_t *hand);
uint score_hand(hand_t *hand);
void play_hand(deck_t* deck);

#endif
