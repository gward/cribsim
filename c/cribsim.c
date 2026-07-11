#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#include "cards.h"
#include "log.h"
#include "play.h"

static void usage(FILE *out) {
    fprintf(out,
            "usage: cribsim [-n GAMES] [--strategy-a PEG,DISCARD] [--strategy-b PEG,DISCARD]\n"
            "\n"
            "Options:\n"
            "  -n GAMES            number of games to play (default: 100)\n"
            "  --strategy-a SPEC   strategy for player A (default: low,simple)\n"
            "  --strategy-b SPEC   strategy for player B (default: low,simple)\n"
            "  -h, --help          show this help\n"
            "\n"
            "PEG:     low | high\n"
            "DISCARD: simple | random\n"
            "\n"
            "Examples:\n"
            "  cribsim -n 1000 --strategy-a high,simple --strategy-b low,random\n");
}

int main(int argc, char *argv[]) {
    log_set_level(LOG_INFO);

    int ngames = 100;
    strategy_t strategy_a = {
        .peg_func = peg_select_low,
        .discard_func = discard_simple,
    };
    strategy_t strategy_b = {
        .peg_func = peg_select_low,
        .discard_func = discard_simple,
    };

    static struct option long_options[] = {
        {"games",      required_argument, 0, 'n'},
        {"strategy-a", required_argument, 0, 'a'},
        {"strategy-b", required_argument, 0, 'b'},
        {"help",       no_argument,       0, 'h'},
        {0, 0, 0, 0},
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "n:h", long_options, NULL)) != -1) {
        switch (opt) {
        case 'n':
            ngames = atoi(optarg);
            if (ngames <= 0) {
                fprintf(stderr, "error: games must be a positive integer\n");
                exit(1);
            }
            break;
        case 'a': {
            char *err = parse_strategy(optarg, &strategy_a);
            if (err != NULL) {
                fprintf(stderr, "error: --strategy-a: %s\n", err);
                exit(1);
            }
            break;
        }
        case 'b': {
            char *err = parse_strategy(optarg, &strategy_b);
            if (err != NULL) {
                fprintf(stderr, "error: --strategy-b: %s\n", err);
                exit(1);
            }
            break;
        }
        case 'h':
            usage(stdout);
            return 0;
        default:
            usage(stderr);
            return 1;
        }
    }

    time_t now = time(NULL);
    pid_t pid = getpid();
    uint seed = (int) now ^ (int) pid ^ ((int) pid << 16);
    log_trace("now = %ld, pid = %d, seed = %ud", now, pid, seed);
    srand(seed);

    int games_won[2] = {0, 0};
    deck_t *deck = new_deck();

    for (int gidx = 0; gidx < ngames; gidx++) {
        playername_t winner = play_game(deck, strategy_a, strategy_b);
        games_won[winner]++;
    }
    log_info("player a: %d wins, player b: %d wins",
             games_won[PLAYER_A],
             games_won[PLAYER_B]);

    free(deck);
    return 0;
}
