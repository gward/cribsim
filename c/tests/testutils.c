#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cards.h"

/* Parse a string like "A♥ 3♥ 5♠ 6♦" into cards, and use it to populate hand. */
void parse_hand(hand_t *dest, char cards[]) {
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
