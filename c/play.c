#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "play.h"
#include "twiddle.h"

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


static uint _rank_value[] = {
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

typedef struct {
    hand_t *hand;
    uint num_15s;
} count_data_t;

/* Examine a single subset of a hand, counting whether it sums to 15. */
void visit_count_15s(int ncards, int indexes[], void* _data) {
    count_data_t* data = (count_data_t*) _data;
    uint sum = 0;
    for (int i = 0; i < ncards; i++) {
        sum += _rank_value[data->hand->cards[indexes[i]].rank];
    }
    if (sum == 15) {
        data->num_15s++;
    }
}

uint count_15s(hand_t *hand) {
    count_data_t data = {hand: hand, num_15s: 0};
    for (int subset_len = hand->ncards; subset_len >= 2; subset_len--) {
        iter_combos(hand->ncards, subset_len, visit_count_15s, &data);
    }
    return data.num_15s;
}

uint count_pairs(hand_t* hand) {
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
    return num_pairs;
}

/* Calculate the score of a single hand (which might have any number
 * of cards). hand must already be sorted!
 */
uint score_hand(hand_t *hand) {
    assert(hand->ncards > 0);

    print_cards("scoring hand", hand->ncards, hand->cards);
    uint num_15s = count_15s(hand);
    uint num_pairs = count_pairs(hand);

    return (num_15s * 2) + (num_pairs * 2);
}

typedef struct {
    uint top_score;
    hand_t *input;
    hand_t *candidate;
    hand_t *winner;
} discard_data_t;

void eval_candidate_simple(int ncards, int indexes[], void* _data) {
    discard_data_t* data = (discard_data_t*) _data;
    hand_t* input = data->input;
    hand_t* candidate = data->candidate;
    hand_t* winner = data->winner;

    assert(candidate->ncards <= input->ncards);
    assert(winner->ncards == candidate->ncards);
    assert(candidate->ncards >= ncards);
    assert(winner->ncards >= ncards);

    printf("visit_candidate: {");
    for (int i = 0; i < ncards; i++) {
        printf("%d%c", indexes[i], (i == ncards - 1) ? '}' : ',');
    }
    putchar('\n');

    for (int i = 0; i < ncards; i++) {
        candidate->cards[i] = input->cards[indexes[i]];
    }
    print_cards("candidate hand", candidate->ncards, candidate->cards);

    uint score = score_hand(candidate);
    if (score > data->top_score) {
        // Ignore ties -- just use the first candidate to get to the top.
        printf("new winner: top_score = %d, score = %d\n",
               data->top_score,
               score);
        data->top_score = score;
        copy_hand(winner, candidate);
    }
    else {
        printf("no change: top_score = %d, score = %d\n",
               data->top_score,
               score);
    }

}


/* Discard two cards that maximize the fixed score -- i.e. the score
 * from the 4 cards kept, ignoring the up card.
 */
void discard_simple(hand_t* hand) {
    sort_cards(hand->ncards, hand->cards);
    print_cards("discard_simple(): sorted hand", hand->ncards, hand->cards);

    hand_t* candidate = new_hand(4);
    hand_t* winner = new_hand(4);

    discard_data_t data = {0, hand, candidate, winner};
    iter_combos(hand->ncards, 4, eval_candidate_simple, (void*) &data);

    // In case every candidate had score 0, arbitrarily pick the last one.
    if (data.top_score == 0) {
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
