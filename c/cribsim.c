#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "cards.h"
#include "play.h"

int main(int argc, char *argv[]) {
    time_t now = time(NULL);
    pid_t pid = getpid();
    uint seed = (int) now ^ (int) pid ^ ((int) pid << 16);
    printf("now = %ld, pid = %d, seed = %ud\n", now, pid, seed);
    srand(seed);

    deck_t *deck = new_deck();
    shuffle_deck(deck);
    print_cards("shuffled deck", deck->ncards, deck->cards);

    play_hand(deck);

    free(deck);

    return 0;
}
