#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "cards.h"
#include "log.h"
#include "play.h"

int main(int argc, char *argv[]) {
    log_set_level(LOG_INFO);

    time_t now = time(NULL);
    pid_t pid = getpid();
    uint seed = (int) now ^ (int) pid ^ ((int) pid << 16);
    log_trace("now = %ld, pid = %d, seed = %ud", now, pid, seed);
    srand(seed);

    int ngames = 100;
    int games_won[2] = {0, 0};
    deck_t *deck = new_deck();

    for (int gidx = 0; gidx < ngames; gidx++) {
        playername_t winner = play_game(deck);
        games_won[winner]++;
    }
    log_info("player a: %d wins, player b: %d wins",
             games_won[PLAYER_A],
             games_won[PLAYER_B]);

    free(deck);
    return 0;
}
