#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "play.h"

/* Drop one card from hand (modify hand in place). */
void drop_one(hand_t* hand, int drop) {
    card_t keep_cards[6];

    for (int src = 0, dst = 0; src < hand->ncards; src++) {
        if (src != drop) {
            keep_cards[dst++] = hand->cards[src];
        }
    }
    hand->ncards--;
    memcpy(hand->cards, keep_cards, sizeof(card_t) * hand->ncards);
}

/* Copy src_hand into dest_hand, dropping two cards along the way. */
void drop_two(hand_t *dest_hand, hand_t *src_hand, int drop1, int drop2) {
    assert(dest_hand->ncards >= src_hand->ncards - 2);
    for (int src = 0, dst = 0; src < src_hand->ncards; src++) {
        if (src != drop1 && src != drop2) {
            dest_hand->cards[dst++] = src_hand->cards[src];
        }
    }
    dest_hand->ncards = src_hand->ncards - 2;
}


/* Calculate the score of a single hand (which might have any number
 * of cards). hand must already be sorted!
 */
uint score_hand(hand_t *hand) {
    assert(hand->ncards > 0);

    //char buf1[5], buf2[5];
    uint score = 0;

    // Search for pairs.
    //
    //  2♦ 3♥ 3♠ 5♠ -> 1 pair
    //  2♦ 2♥ 5♣ 5♠ -> 2 pairs
    //  2♦ 2♥ 2♠ 5♠ -> 3 pairs
    uint ncards = hand->ncards;
    card_t* cards = hand->cards;
    int num_pairs = 0;
    for (int i = 0; i < ncards - 1; i++) {
        for (int j = i + 1; j < ncards && cards[j].rank == cards[i].rank; j++) {
            num_pairs++;
        }
    }
    score += num_pairs * 2;

    return score;
}

/* Discard two cards that maximize the fixed score -- i.e. the score
 * from the 4 cards kept, ignoring the up card.
 */
void discard_simple(hand_t* hand) {
    sort_cards(hand->ncards, hand->cards);
    print_cards("discard_simple(): sorted hand", hand->ncards, hand->cards);

    uint score, top_score = 0;
    hand_t* candidate = new_hand(hand->ncards);
    hand_t* winner = new_hand(hand->ncards);
    for (int i = 0; i < hand->ncards; i++) {
        for (int j = i + 1; j < hand->ncards; j++) {
            drop_two(candidate, hand, i, j);
            printf("dropped i=%d, j=%d: candidate hand:\n", i, j);
            print_cards(NULL, candidate->ncards, candidate->cards);

            score = score_hand(candidate);
            if (score > top_score) {
                // ignore ties -- we'll just use the last candidate hand
                // that beats the current top score
                printf("new winner: top_score = %d, score = %d\n",
                       top_score,
                       score);
                top_score = score;
                copy_hand(winner, candidate);
            }
            else {
                printf("no change: top_score = %d, score = %d\n",
                       top_score,
                       score);
            }
        }
    }

    // In case every permutation has score 0, arbitrarily pick the last one.
    if (top_score == 0) {
        copy_hand(winner, candidate);
    }

    print_cards("winning candidate", winner->ncards, winner->cards);
    copy_hand(hand, winner);

    free(winner);
    free(candidate);
}

/* Discard two cards at random. */
void discard_random(hand_t* hand) {
    int drop1, drop2;
    drop1 = rand() % hand->ncards;
    while ((drop2 = rand() % hand->ncards) == drop1) {
    }
    printf("discard_random: drop1=%d, drop2=%d\n", drop1, drop2);

    hand_t* tmp_hand = new_hand(hand->ncards - 2);
    /* card_t dest_cards[hand->ncards - 2]; */
    /* hand_t dest_hand; */
    /* dest_hand.ncards = hand->ncards - 2; */
    /* dest_hand.cards = dest_cards; */
    drop_two(tmp_hand, hand, drop1, drop2);
    //hand->ncards = tmp_hand->ncards;
    //memcpy(hand->cards, tmp_hand->cards, sizeof(card_t) * tmp_hand->ncards);
    copy_hand(hand, tmp_hand);
    free(tmp_hand);

    /*
    int drop;

    drop = rand() % hand->ncards;
    printf("discard_random: ncards = %d, drop = %d\n", hand->ncards, drop);
    drop_one(hand, drop);

    drop = rand() % hand->ncards;
    printf("discard_random: ncards = %d, drop = %d\n", hand->ncards, drop);
    drop_one(hand, drop);
    */
}

void play_hand(deck_t* deck) {
    int nplayers = 2;
    int ncards = 6;

    hand_t* hand_a = new_hand(ncards);
    hand_t* hand_b = new_hand(ncards);
    printf("hand_a = %p %d\n", hand_a, hand_a->ncards);
    printf("hand_b = %p %d\n", hand_b, hand_b->ncards);

    // Deal the hands.
    int deck_offset = 0;
    for (int i = 0; i < ncards; i++) {
        hand_a->cards[i] = deck->cards[deck_offset++];
        hand_b->cards[i] = deck->cards[deck_offset++];
    }

    print_cards("hand_a after deal", hand_a->ncards, hand_a->cards);
    print_cards("hand_b after deal", hand_b->ncards, hand_b->cards);
    assert(deck_offset == ncards * nplayers);
    deck->offset = deck_offset;

    // Play two different strategies for discarding cards.
    discard_simple(hand_a);
    discard_random(hand_b);
    print_cards("hand_a after discard", hand_a->ncards, hand_a->cards);
    print_cards("hand_b after discard", hand_b->ncards, hand_b->cards);

    free(hand_b);
    free(hand_a);
}
