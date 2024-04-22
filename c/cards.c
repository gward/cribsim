#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cards.h"

/* Map member of the rank_t enum to the value of that card when
 * counting 15s or pegging.
 */
uint rank_value[] = {
    0,                        /* RANK_JOKER */
    1,                        /* RANK_ACE */
    2,                        /* RANK_2 */
    3,                        /* RANK_3 */
    4,                        /* RANK_4 */
    5,                        /* RANK_5 */
    6,                        /* RANK_6 */
    7,                        /* RANK_7 */
    8,                        /* RANK_8 */
    9,                        /* RANK_9 */
    10,                       /* RANK_10 */
    10,                       /* RANK_JACK */
    10,                       /* RANK_QUEEN */
    10,                       /* RANK_KING */
};

/* Write a debug-friendly string representation of a card to result.
 * result must have room for 5 chars (including trailing NUL byte).
 * Return result.
 */
char* card_debug(char result[], card_t card) {
    sprintf(result, "%02d:%d", (int) card.rank, (int) card.suit);
    return result;
}

/* Write a human-friendly string representation of a card to result.
 * result must have room for 5 chars (including trailing NUL byte).
 * Return result.
 */
char* card_str(char result[], card_t card) {
    char rank_char[] = "*A234567890JQK";
    result[0] = rank_char[card.rank];
    switch (card.suit) {
    case SUIT_NONE:
        result[1] = 0;
        break;
    case SUIT_CLUB:
        strcpy(result+1, "♣");
        break;
    case SUIT_DIAMOND:
        strcpy(result+1, "♦");
        break;
    case SUIT_HEART:
        strcpy(result+1, "♥");
        break;
    case SUIT_SPADE:
        strcpy(result+1, "♠");
        break;
    }
    result[4] = 0;

    return result;
}

void print_cards(char *prefix, int ncards, card_t cards[]) {
    char debug_str[5];
    char friendly_str[5];
    if (prefix != NULL) {
        printf("%s:\n", prefix);
    }
    for (int i = 0; i < ncards; i++) {
        printf("%2d: %s = %s\n",
               i,
               card_debug(debug_str, cards[i]),
               card_str(friendly_str, cards[i]));
    }
}

static int cmp_cards(const void* a, const void* b) {
    card_t* card_a = (card_t*) a;
    card_t* card_b = (card_t*) b;
    /* char card_a_str[5], card_b_str[5]; */
    /* printf("cmp_cards(): card_a = %s, card_b = %s\n", */
    /*        card_debug(card_a_str, *card_a), */
    /*        card_debug(card_b_str, *card_b)); */

    uint val_a = (card_a->rank << 4) | (card_a->suit);
    uint val_b = (card_b->rank << 4) | (card_b->suit);
    int cmp = val_a - val_b;
    return cmp;
}

/* Sort an array of cards in place (by rank then suit). */
void sort_cards(int ncards, card_t cards[]) {
    qsort(cards, ncards, sizeof(card_t), cmp_cards);
}

/* allocate and populate a new deck with the standard 52 cards, sorted */
deck_t* new_deck() {
    // sanity check to ensure I understand struct layout
    assert(sizeof(card_t) == sizeof(uint));

    int ncards = 52;
    deck_t* deck = malloc(sizeof(deck_t) + (ncards * sizeof(card_t)));
    deck->offset = 0;
    deck->ncards = ncards;
    int i = 0;
    for (int rank = RANK_ACE; rank <= RANK_KING; rank++) {
        for (int suit = SUIT_CLUB; suit <= SUIT_SPADE; suit++) {
            deck->cards[i].rank = rank;
            deck->cards[i].suit = suit;
            i++;
        }
    }
    return deck;
}

/* shuffle an existing deck in place */
void shuffle_deck(deck_t* deck) {
    card_t tmp;
    for (int i = 0; i < deck->ncards; i++) {
        int j = rand() % deck->ncards;
        tmp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = tmp;
    }
}

/* Allocate an empty hand of the requested size */
hand_t* new_hand(int ncards) {
    printf("sizeof(hand_t) = %lu\n", sizeof(hand_t));
    printf("sizeof(card_t) = %lu\n", sizeof(card_t));
    printf("ncards = %d\n", ncards);

    int nbytes = sizeof(hand_t) + (ncards * sizeof(card_t));
    printf("new_hand: nbytes = %d\n", nbytes);
    hand_t* hand = calloc(1, nbytes);
    hand->ncards = ncards;
    return hand;
}

void copy_hand(hand_t *dest, hand_t *src) {
    assert(dest->ncards >= src->ncards);
    dest->ncards = src->ncards;
    memcpy(dest->cards, src->cards, sizeof(card_t) * src->ncards);
}

void hand_set_card(hand_t *hand, int idx, rank_t rank, suit_t suit) {
    assert(hand->ncards > idx);
    hand->cards[idx].rank = rank;
    hand->cards[idx].suit = suit;
}
