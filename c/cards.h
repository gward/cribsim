#ifndef _CARDS_H
#define _CARDS_H

typedef unsigned int uint;

typedef enum {
    SUIT_NONE,
    SUIT_CLUB,
    SUIT_DIAMOND,
    SUIT_HEART,
    SUIT_SPADE,
} suit_t;

typedef enum {
    RANK_JOKER,
    RANK_ACE,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_9,
    RANK_10,
    RANK_JACK,
    RANK_QUEEN,
    RANK_KING,
} rank_t;

typedef struct {
    suit_t suit:4;
    rank_t rank:4;
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

extern uint rank_value[14];

char *card_debug(char result[], card_t card);
char *card_str(char result[], card_t card);
void print_cards(char *prefix, int ncards, card_t cards[]);
void sort_cards(int ncards, card_t cards[]);

hand_t *new_hand(int ncards);
void copy_hand(hand_t *dest, hand_t *src);
void hand_set_card(hand_t *hand, int idx, rank_t rank, suit_t suit);

deck_t *new_deck();
void shuffle_deck(deck_t *deck);

#endif
