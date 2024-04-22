#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "cards.h"
#include "play.h"

int main(int argc, char *argv[]) {
    uint seed = (uint) getpid();
    seed ^= (uint) time(NULL);
    srand(seed);

    deck_t *deck = new_deck();
    shuffle_deck(deck);
    print_cards("shuffled deck", deck->ncards, deck->cards);

    play_hand(deck);

    free(deck);

    return 0;
}
