#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cards.h"
#include "log.h"
#include "stringbuilder.h"

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
char *card_debug(char result[], card_t card) {
    sprintf(result, "%02d:%d", (int) card.rank, (int) card.suit);
    return result;
}

/* Write a human-friendly string representation of a card to result.
 * result must have room for 5 chars (including trailing NUL byte).
 * Return result.
 */
char *card_str(char result[], card_t card) {
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

int card_cmp(card_t *card_a, card_t *card_b) {
    unsigned char val_a = *((unsigned char *) (card_a));
    unsigned char val_b = *((unsigned char *) (card_b));
    return val_a - val_b;
}

void log_cards(int level, char *prefix, int ncards, card_t cards[]) {
    char debug_str[5];
    char friendly_str[5];

    stringbuilder_t sb;
    sb_init(&sb, 64);

    if (prefix != NULL) {
        sb_append(&sb, prefix);
        sb_append(&sb, ":\n");
    }

    for (int i = 0; i < ncards; i++) {
        sb_printf(&sb,
                  "%2d: %s = %s\n",
                  i,
                  card_debug(debug_str, cards[i]),
                  card_str(friendly_str, cards[i]));
    }

    char *out = sb_as_string(&sb);
    out[strlen(out) - 1] = 0;      // kill final newline
    log_log(level, __FILE__, __LINE__, out);
    sb_close(&sb);
}

static int cmp_cards(const void *a, const void *b) {
    return card_cmp((card_t *) a,  (card_t *) b);
}

/* Sort an array of cards in place (by rank then suit). */
void sort_cards(int ncards, card_t cards[]) {
    qsort(cards, ncards, sizeof(card_t), cmp_cards);
}

/* allocate and populate a new deck with the standard 52 cards, sorted */
deck_t *new_deck() {
    // sanity check to ensure I understand struct layout
    assert(sizeof(card_t) == sizeof(uint));

    int ncards = 52;
    deck_t *deck = malloc(sizeof(deck_t) + (ncards * sizeof(card_t)));
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
void shuffle_deck(deck_t *deck) {
    card_t tmp;
    for (int i = 0; i < deck->ncards; i++) {
        int j = rand() % deck->ncards;
        tmp = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = tmp;
    }
}

/* Allocate an empty hand of the requested size */
hand_t *new_hand(int size) {
    int nbytes = sizeof(hand_t) + (size * sizeof(card_t));
    hand_t *hand = calloc(1, nbytes);
    hand->size = size;
    hand->starter = -1;
    return hand;
}

char *hand_str(char buf[], size_t size, hand_t *hand) {
    size_t size_left = size;
    char *cur = buf;
    for (int i = 0; i < hand->ncards; i++) {
        if (size_left < 5) {
            break;
        }

        card_str(cur, hand->cards[i]);
        cur += 4;
        size_left -= 4;
        if (i < hand->ncards - 1) {
            *(cur++) = ' ';
            size_left--;
        }
    }
    assert(size_left >= 1);
    *(cur++) = 0;
    size_left--;
    return buf;
}

void hand_append(hand_t *dest, card_t card) {
    assert(dest->ncards < dest->size);
    dest->cards[dest->ncards] = card;
    dest->ncards++;
}

void hand_truncate(hand_t *dest) {
    dest->ncards = 0;
    memset(dest->cards, 0, sizeof(card_t) * dest->size);
}

void hand_delete(hand_t *dest, int del_idx) {
    assert(del_idx < dest->ncards);
    int i;
    for (i = del_idx; i < dest->ncards - 1; i++) {
        dest->cards[i] = dest->cards[i+1];
    }
    dest->cards[i] = (card_t) {0, 0};
    dest->ncards--;
}

void copy_hand(hand_t *dest, hand_t *src) {
    assert(dest->size >= src->ncards);
    dest->ncards = src->ncards;
    memcpy(dest->cards, src->cards, sizeof(card_t) * src->ncards);
}

void hand_set_card(hand_t *hand, int idx, rank_t rank, suit_t suit) {
    assert(hand->ncards > idx);
    hand->cards[idx].rank = rank;
    hand->cards[idx].suit = suit;
}
