#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "cards.h"
#include "log.h"
#include "play.h"

int main(int argc, char *argv[]) {
    log_set_level(LOG_DEBUG);

    time_t now = time(NULL);
    pid_t pid = getpid();
    uint seed = (int) now ^ (int) pid ^ ((int) pid << 16);
    log_trace("now = %ld, pid = %d, seed = %ud", now, pid, seed);
    srand(seed);

    game_config_t game_config = game_config_init();
    game_state_t game_state = game_state_init();

    deck_t *deck = new_deck();
    shuffle_deck(deck);

    // Players A and B have the same naive pegging strategy.
    // But Player A has a better discard strategy.
    strategy_t strategy_a = {
        peg_func: peg_select_low,
        discard_func: discard_simple,
    };
    strategy_t strategy_b = {
        peg_func: peg_select_low,
        discard_func: discard_random,
    };
    uint score_a = 0;
    uint score_b = 0;

    // First hand: A is dealer (player 1), so B is player 0.
    game_config.strategy[0] = strategy_b;
    game_config.strategy[1] = strategy_a;

    play_hand(game_config, &game_state, deck);
    score_b = game_state.scores[0];
    score_a = game_state.scores[1];
    log_info("after one hand: scores={%d, %d}, score_a=%d, score_b=%d winner=%d",
             game_state.scores[0],
             game_state.scores[1],
             score_a,
             score_b,
             game_state.winner);

    free(deck);

    return 0;
}
