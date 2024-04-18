#ifndef _CARDS_H
#define _CARDS_H

typedef unsigned int uint;

typedef enum {
    CLUB,
    DIAMOND,
    HEART,
    SPADE,
} suit_t;

typedef struct {
    uint suit:2;
    uint rank:4;
} card_t;

typedef struct {
    uint ncards;
    card_t cards[];
} hand_t;

typedef struct {
    uint offset;    // next card to deal; 0 means full deck
    uint ncards;
    card_t cards[];
} deck_t;

char* card_debug(char result[], card_t card);
char* card_str(char result[], card_t card);
void print_cards(char *prefix, int ncards, card_t cards[]);
void sort_cards(int ncards, card_t cards[]);

hand_t* new_hand(int ncards);
void copy_hand(hand_t *dest, hand_t *src);

deck_t* new_deck();
void shuffle_deck(deck_t* deck);

#endif
