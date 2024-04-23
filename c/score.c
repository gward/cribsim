#include <assert.h>
#include <stdio.h>

#include "cards.h"
#include "twiddle.h"

typedef struct {
    hand_t *hand;
    uint num_15s;
} count_data_t;

/* Examine a single subset of a hand, counting whether it sums to 15. */
void visit_count_15s(int ncards, int indexes[], void *_data) {
    count_data_t *data = (count_data_t *) _data;
    uint sum = 0;
    for (int i = 0; i < ncards; i++) {
        sum += rank_value[data->hand->cards[indexes[i]].rank];
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

uint count_pairs(hand_t *hand) {
    // Search for pairs.
    //
    //  2♦ 3♥ 3♠ 5♠ -> 1 pair
    //  2♦ 2♥ 5♣ 5♠ -> 2 pairs
    //  2♦ 2♥ 2♠ 5♠ -> 3 pairs
    uint ncards = hand->ncards;
    card_t *cards = hand->cards;
    int num_pairs = 0;
    for (int i = 0; i < ncards - 1; i++) {
        for (int j = i + 1; j < ncards && cards[j].rank == cards[i].rank; j++) {
            num_pairs++;
        }
    }
    return num_pairs;
}

uint count_runs(hand_t *hand) {
    uint run_points = 0;
    uint current_run = 1;
    uint repeats = 1;
    int i;
    for (i = 1; i < hand->ncards; i++) {
        rank_t prev_rank = hand->cards[i-1].rank;
        rank_t cur_rank = hand->cards[i].rank;

        // 3 4: extend current_run to 2
        // 3 4 5: extend current_run to 3
        if (prev_rank < RANK_KING && cur_rank == prev_rank + 1) {
            current_run++;
        }

        // 3 3: if we see 4 5 next, it's a double run (repeats = 2)
        // 3 4 4: if we see 5 next, it's a double run (repeats = 2)
        // 3 4 4 5 5: it's a double double run (repeats = 4)
        else if (prev_rank == cur_rank) {
            repeats *= 2;
        }

        // End of a run (if any).
        else if (prev_rank < RANK_QUEEN && cur_rank > prev_rank + 1) {
            if (current_run >= 3) {
                run_points += current_run * repeats;
                printf("run ended at i=%d: current_run=%d, repeats=%d, run_points=%d\n",
                       i,
                       current_run,
                       repeats,
                       run_points);
            }
            current_run = 1;
            repeats = 1;
        }
    }

    // Fall off the end also counts as the end of a run.
    if (current_run >= 3) {
        run_points += current_run * repeats;
        printf("run ended at i=%d: current_run=%d, repeats=%d, run_points=%d\n",
               i,
               current_run,
               repeats,
               run_points);
    }

    return run_points;
}

uint count_flush(hand_t *hand) {
    // If we're doing something weird, ignore flushes.
    /* if (hand->ncards < 4 || hand->ncards > 5) { */
    /*     return 0; */
    /* } */

    // If everything except the starter is of the same suite, that's a flush.
    suit_t flush_suit = SUIT_NONE;
    uint cur_flush = 0;
    for (int i = 0; i < hand->ncards; i++) {
        if (hand->starter > -1 && i == hand->starter) {
            // Skip the starter card, no matter where we are in the hand.
            continue;
        }
        else if (flush_suit == SUIT_NONE) {
            // This is the first non-starter card: flush of length 1.
            flush_suit = hand->cards[i].suit;
            cur_flush = 1;
            continue;
        }

        suit_t cur_suit = hand->cards[i].suit;
        if (cur_suit != flush_suit) {
            // The flush is broken: we're not going to find anything.
            return 0;
        }
        else if (cur_flush > 0 && cur_suit == flush_suit) {
            // Continue the flush.
            cur_flush++;
        }
    }

    // If the starter card is known, it can extend a flush by one.
    if (hand->starter > -1 && cur_flush > 0) {
        card_t starter = hand->cards[hand->starter];
        if (starter.suit == flush_suit) {
            cur_flush++;
        }
    }

    return cur_flush;
}

/* Calculate the score of a single hand (which might have any number
 * of cards). hand must already be sorted!
 */
uint score_hand(hand_t *hand) {
    assert(hand->ncards > 0);

    print_cards("scoring hand", hand->ncards, hand->cards);
    uint num_15s = count_15s(hand);
    uint num_pairs = count_pairs(hand);
    uint run_points = count_runs(hand);
    uint flush_points = count_flush(hand);

    return (num_15s * 2) + (num_pairs * 2) + run_points + flush_points;
}
