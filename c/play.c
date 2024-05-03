#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "play.h"
#include "score.h"
#include "stringbuilder.h"
#include "twiddle.h"

gamestate_t gamestate_init() {
    return (gamestate_t) {
        player_name: {PLAYER_A, PLAYER_B},
        strategy: {
            (strategy_t) {peg_func: NULL, discard_func: NULL},
            (strategy_t) {peg_func: NULL, discard_func: NULL},
        },
        score: {0, 0},
        winner: -1,
    };
}

char playername_as_char(playername_t name) {
    // Simple arithmetic because PLAYER_A = 0, PLAYER_B = 1.
    return (char) name + 'a';
}

/* Drop one card from hand (modify hand in place). */
void drop_one(hand_t *hand, int drop) {
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
    assert(dest_hand->size >= src_hand->ncards - 2);
    for (int src = 0, dst = 0; src < src_hand->ncards; src++) {
        if (src != drop1 && src != drop2) {
            dest_hand->cards[dst++] = src_hand->cards[src];
        }
    }
    dest_hand->ncards = src_hand->ncards - 2;
}

typedef struct {
    uint top_score;
    hand_t *input;
    hand_t *candidate;
    hand_t *winner;
} discard_data_t;

void eval_candidate_simple(int ncards, int indexes[], void *_data) {
    discard_data_t *data = (discard_data_t*) _data;
    hand_t *input = data->input;
    hand_t *candidate = data->candidate;
    hand_t *winner = data->winner;

    assert(candidate->size <= input->ncards);
    assert(winner->size == candidate->size);
    assert(candidate->size == ncards);
    assert(winner->size >= ncards);

    size_t size = 50;
    int offset = 0;
    char buf[size];
    strncpy(buf, "eval_candidate_simple: {", size);
    offset = strlen(buf);
    for (int i = 0; i < ncards; i++) {
        int nbytes = snprintf(buf + offset, size - offset, "%d", indexes[i]);
        assert(nbytes < size - offset);
        offset += nbytes;
        if (i < ncards - 1) {
            assert(size - offset >= 1);
            buf[offset] = ',';
            offset++;
        }
    }
    assert(size - offset >= 1);
    strncpy(buf + offset, "}", size - offset);
    log_trace(buf);

    hand_truncate(candidate);
    for (int i = 0; i < ncards; i++) {
        hand_append(candidate, input->cards[indexes[i]]);
    }
    log_cards(LOG_TRACE, "candidate hand", candidate->ncards, candidate->cards);

    score_t score = score_hand(candidate);
    if (score.total > data->top_score) {
        // Ignore ties -- just use the first candidate to get to the top.
        log_trace("new winner: top_score = %d, score = %d",
                  data->top_score,
                  score.total);
        data->top_score = score.total;
        copy_hand(winner, candidate);
    }
    else {
        log_trace("no change: top_score = %d, score = %d",
                  data->top_score,
                  score.total);
    }
}

/* Discard two cards that maximize the fixed score -- i.e. the score
 * from the 4 cards kept, ignoring the starter card.
 */
void discard_simple(hand_t *hand, hand_t *crib) {
    hand_t *candidate = new_hand(4);
    hand_t *winner = new_hand(4);

    discard_data_t data = {0, hand, candidate, winner};
    iter_combos(hand->ncards, 4, eval_candidate_simple, (void *) &data);

    // In case every candidate had score 0, arbitrarily pick the last one.
    if (data.top_score == 0) {
        copy_hand(winner, candidate);
    }

    // Add discarded cards to the crib.
    for (int i = 0; i < hand->ncards; i++) {
        bool kept = false;
        for (int j = 0; j < winner->ncards; j++) {
            if (card_cmp(&hand->cards[i], &winner->cards[j]) == 0) {
                kept = true;
                break;
            }
        }
        if (!kept) {
            hand_append(crib, hand->cards[i]);
        }
    }

    log_cards(LOG_TRACE, "winning candidate", winner->ncards, winner->cards);
    copy_hand(hand, winner);

    free(winner);
    free(candidate);
}

/* Discard two cards at random. */
void discard_random(hand_t *hand, hand_t *crib) {
    int drop1, drop2;
    drop1 = rand() % hand->ncards;
    while ((drop2 = rand() % hand->ncards) == drop1) {
    }
    log_trace("discard_random: drop1=%d, drop2=%d", drop1, drop2);

    hand_append(crib, hand->cards[drop1]);
    hand_append(crib, hand->cards[drop2]);

    hand_t *tmp_hand = new_hand(hand->ncards - 2);
    drop_two(tmp_hand, hand, drop1, drop2);
    copy_hand(hand, tmp_hand);
    free(tmp_hand);
}

peg_state_t *new_peg_state(int ncards) {
    peg_state_t *peg = (peg_state_t *) calloc(1, sizeof(peg_state_t));
    peg->num_rounds = 0;
    peg->avail[0] = new_hand(ncards);
    peg->avail[1] = new_hand(ncards);
    peg->cur_played = new_hand(ncards * 2);
    memset(peg->cards_played, 0, sizeof(hand_t *) * MAX_ROUNDS);

    return peg;
}

void peg_state_free(peg_state_t *peg) {
    free(peg->avail[0]);
    free(peg->avail[1]);
    if (peg->cur_played != NULL) {
        free(peg->cur_played);
    }
    for (int i = 0; i < MAX_ROUNDS; i++) {
        if (peg->cards_played[i] != NULL) {
            free(peg->cards_played[i]);
        }
    }
    free(peg);
}

// Naive pegging strategy: select the smallest available card, as long
// as we don't go over 31. Assumes the arrays of available cards are
// sorted.
int peg_select_low(peg_state_t *peg, int player, int other) {
    hand_t *avail = peg->avail[player];
    card_t card = avail->cards[0];
    if (peg->cur_count + rank_value[card.rank] <= 31) {
        return 0;
    }
    return -1;
}

// Naive pegging strategy: select the largest available card that
// doesn't go over 31. Assumes the arrays of available cards are
// sorted.
int peg_select_high(peg_state_t *peg, int player, int other) {
    hand_t *avail = peg->avail[player];
    for (int i = avail->ncards - 1; i >= 0; i--) {
        card_t card = avail->cards[i];
        uint count = peg->cur_count + rank_value[card.rank];
        if (count <= 31) {
            return i;
        }
    }

    // Ran out of cards before we got under 31. Cannot play.
    return -1;
}

uint peg_count_pairs(peg_state_t *peg, int player) {
    uint same_rank = 1;
    uint pair_points = 0;
    card_t last_played = peg->cur_played->cards[peg->cur_played->ncards - 1];
    for (int i = peg->cur_played->ncards - 2; i >= 0; i--) {
        card_t played = peg->cur_played->cards[i];
        if (played.rank == last_played.rank) {
            same_rank++;
            if (same_rank == 2) {
                pair_points = 2;
            }
            else {
                pair_points *= same_rank;     // 3-of-a-kind = 6, 4-of-a-kind = 12
            }
        }
        else {
            break;
        }
    }
    if (pair_points > 0) {
        log_trace("  found %d-of-a-kind, %d points to player %d",
                  same_rank,
                  pair_points,
                  player);
    }
    return pair_points;
}

uint peg_count_runs(peg_state_t *peg, int player) {
    // cur_played = {4, 6, 8, 7}       : run of 3
    // cur_played = {A, 3, 5, 7, 8}    : no runs
    // cur_played = {3, A, 5, 7, 8, 6} : run of 4
    // cur_played = {2, 5, 6, 4}       : run of 3
    // cur_played = {2, 5, 6, 4, 7}    : run of 4 (preceding run already counted)
    // cur_played = {A, 2, 3, 4, 5, 6, 7}: run of 7 (longest possible)

    uint max_run = 7;
    if (peg->cur_played->ncards < max_run) {
        max_run = peg->cur_played->ncards;
    }

    card_t *after = peg->cur_played->cards + peg->cur_played->ncards;
    card_t candidate[max_run];
    for (int len = max_run; len >= 3; len--) {
        memcpy(candidate, after - len, len * sizeof(card_t));
        sort_cards(len, candidate);

        int found_run = len;
        rank_t prev_rank = candidate[0].rank;
        for (int i = 1; i < len; i++) {
            if (prev_rank == RANK_KING || candidate[i].rank != prev_rank + 1) {
                found_run = 0;
            }
            prev_rank = candidate[i].rank;
        }
        if (found_run > 0) {
            log_trace("  found run of %d: %d points to player %d",
                      found_run,
                      found_run,
                      player);
            return found_run;
        }
    }
    return 0;
}

bool peg_hands(int nplayers,
               peg_state_t *peg,
               hand_t *hands[],
               peg_func_t select[],
               game_callback_func_t callback,
               void *cb_data) {
    assert(nplayers == 2);
    assert(hands[0]->ncards == hands[1]->ncards);
    uint ncards = hands[0]->ncards;
    int bufsize = ncards * 5;
    char buf1[bufsize], buf2[bufsize];

    copy_hand(peg->avail[0], hands[0]);
    copy_hand(peg->avail[1], hands[1]);

    log_debug("peg_hands(): hands[0]=%s, hands[1]=%s",
              hand_str(buf1, bufsize, hands[0]),
              hand_str(buf2, bufsize, hands[1]));

    // Start pegging from hands[0], aka player 0, aka avail[0]. This implies that
    // player 1 (hands[1]) is the dealer.
    int player = 1;
    int other = 0;

    // Keep track of which players are blocked, i.e. cannot play a card and keep
    // the count at 31 or lower.
    bool blocked[2] = {false, false};
    bool need_reset = false;

    while (true) {
        if (need_reset) {
            // We either hit 31, or both players are blocked (neither could
            // play without going over 31).
            assert(peg->num_rounds < MAX_ROUNDS);
            peg->cards_played[peg->num_rounds] = peg->cur_played;
            peg->counts[peg->num_rounds] = peg->cur_count;
            peg->num_rounds++;
            peg->cur_played = new_hand(ncards * 2);
            peg->cur_count = 0;
            blocked[player] = false;
            blocked[other] = false;
            need_reset = false;
        }

        // Flip to the other player ... unless they are blocked.
        if (!(blocked[other])) {
            other = player;
            player ^= 1;
        }

        uint player_left = peg->avail[player]->ncards;
        uint other_left = peg->avail[other]->ncards;
        uint total_left = player_left + other_left;

        assert(total_left > 0);

        // Current player is out of cards.
        if (player_left == 0) {
            log_trace("  player %d out: try the other player", player);

            if (blocked[other]) {
                log_trace("  but player %d blocked: need reset", other);
                need_reset = true;
            }
            continue;
        }

        // Both players blocked, but at least one has cards left: reset the
        // count and continue pegging.
        if (blocked[player] && blocked[other] && total_left > 0) {
            log_trace("  both players blocked, still have %d cards left: need reset",
                      total_left);
            need_reset = true;
            continue;
        }

        assert(!blocked[player]);

        // Current player selects a card to play -- or decides that they are blocked.
        int selected = select[player](peg, player, other);
        if (selected == -1) {
            log_trace("  player %d says go (is blocked)", player);
            if (!blocked[other]) {
                log_trace("  player %d blocked: 1 point to player %d",
                          player,
                          other);
                peg->points[other]++;
                if (callback(cb_data, other, 1)) {
                    return true;
                }
            }
            else {
                log_trace("  player %d blocked: no points to player %d (already blocked)",
                          player,
                          other);
            }
            blocked[player] = true;
            continue;
        }

        // Play the selected card: move it from the player's 'avail' array to
        // 'played' and 'cur_played', and update the current count.
        assert(selected >= 0);
        assert(selected < peg->avail[player]->ncards);
        card_t card = peg->avail[player]->cards[selected];
        hand_delete(peg->avail[player], selected);
        hand_append(peg->cur_played, card);
        peg->cur_count += rank_value[card.rank];
        assert(peg->cur_count <= 31);       // make sure select() does not break the rules

        // Anything interesting about the count?
        if (peg->cur_count == 15) {
            log_trace("  player %d played card %s, cur_count=%d: 2 points to player %d",
                      player,
                      card_str(buf1, card),
                      peg->cur_count,
                      player);
            peg->points[player] += 2;
            if (callback(cb_data, player, 2)) {
                return true;
            }
        }
        else if (peg->cur_count == 31) {
            if (blocked[other]) {
                log_trace("  player %d played card %s, cur_count=%d: "
                          "1 point to player %d because player %d is blocked",
                          player,
                          card_str(buf1, card),
                          peg->cur_count,
                          player,
                          other);
                peg->points[player] += 1;
                if (callback(cb_data, player, 1)) {
                    return true;
                }
            }
            else if (other_left == 0 && peg->avail[player]->ncards == 0) {
                log_trace("  player %d played card %s, cur_count=%d: 1 point to player %d for last card",
                          player,
                          card_str(buf1, card),
                          peg->cur_count,
                          player);
                peg->points[player] += 1;
                if (callback(cb_data, player, 1)) {
                    return true;
                }
            }
            else {
                log_trace("  player %d played card %s, cur_count=%d: 2 points to player %d",
                          player,
                          card_str(buf1, card),
                          peg->cur_count,
                          player);
                peg->points[player] += 2;
                if (callback(cb_data, player, 2)) {
                    return true;
                }
            }

            if (peg->avail[player]->ncards > 0) {
                need_reset = true;
            }
        }
        else {
            log_trace("  player %d played card %s, cur_count=%d",
                      player,
                      card_str(buf1, card),
                      peg->cur_count);
        }


        log_trace("  need_reset=%d, points={%d, %d}",
                  need_reset,
                  peg->points[0],
                  peg->points[1]);

        // Check for M-of-a-kind.
        uint pair_points = peg_count_pairs(peg, player);
        peg->points[player] += pair_points;
        if (callback(cb_data, player, pair_points)) {
            return true;
        }

        // Check for runs of M.
        uint run_points = peg_count_runs(peg, player);
        peg->points[player] += run_points;
        if (callback(cb_data, player, run_points)) {
            return true;
        }

        // Check for last card.
        if (other_left == 0 && peg->avail[player]->ncards == 0) {
            log_trace("  player %d played last card: 1 point, done pegging", player);
            peg->points[player]++;
            if (callback(cb_data, player, 1)) {
                return true;
            }
            break;
        }
    }

    assert(peg->num_rounds < MAX_ROUNDS);
    peg->cards_played[peg->num_rounds] = peg->cur_played;
    peg->counts[peg->num_rounds] = peg->cur_count;
    peg->num_rounds++;
    for (int i = peg->num_rounds; i < MAX_ROUNDS; i++) {
        peg->counts[i] = -1;
        peg->cards_played[i] = NULL;
    }
    peg->cur_played = NULL;

    assert(peg->avail[0]->ncards == 0);
    assert(peg->avail[1]->ncards == 0);

    log_debug("pegging done: %d points to player 0, %d points to player 1",
              peg->points[0],
              peg->points[1]);
    return false;
}


/* Add the starter card to a hand, sort the hand in-place, and set
 * hand->starter to record where position of 'starter' in 'hand'.
 */
void add_starter(hand_t *hand, card_t starter) {
    hand_append(hand, starter);
    sort_cards(hand->ncards, hand->cards);
    hand->starter = -1;
    for (int i = 0; i < hand->ncards; i++) {
        if (card_cmp(&hand->cards[i], &starter) == 0) {
            hand->starter = i;
            break;
        }
    }
    assert(hand->starter >= 0);
}

bool update_scores(void *data, int player, uint points) {
    gamestate_t *game_state = data;
    playername_t player_name = game_state->player_name[player];
    game_state->score[player_name] += points;
    if (game_state->score[player_name] >= 121) {
        log_info("winner: player %d/%c with %d points",
                 player,
                 playername_as_char(player_name),
                 game_state->score[player_name]);
        game_state->winner = player_name;
        return true;
    }
    return false;
}

bool evaluate_hands(gamestate_t *game_state,
                    int nplayers,
                    hand_t *hands[],
                    hand_t *crib,
                    card_t starter) {
    assert(nplayers == 2);
    assert(hands[0]->ncards == hands[1]->ncards);
    assert(hands[0]->ncards == crib->ncards);

    if (score_starter_jack(starter, update_scores, game_state)) {
        return true;
    }

    playername_t *pname = game_state->player_name;
    peg_func_t peg_funcs[2] = {
        game_state->strategy[pname[0]].peg_func,
        game_state->strategy[pname[1]].peg_func,
    };

    peg_state_t *peg = new_peg_state(hands[0]->ncards);
    bool done = peg_hands(nplayers, peg, hands, peg_funcs, update_scores, game_state);
    peg_state_free(peg);
    if (done) {
        return true;
    }

    // Add starter to each hand and the crib, and score all three.
    add_starter(hands[0], starter);
    add_starter(hands[1], starter);
    add_starter(crib, starter);

    score_t score;

    score = score_hand(hands[0]);
    score_log("hands[0] with starter", score);
    if (update_scores(game_state, 0, score.total)) {
        return true;
    }

    score = score_hand(hands[1]);
    score_log("hands[1] with starter", score);
    if (update_scores(game_state, 1, score.total)) {
        return true;
    }

    score = score_hand(crib);
    score_log("crib with starter ", score);
    if (update_scores(game_state, 1, score.total)) {
        return true;
    }

    return false;
}

bool play_hand(gamestate_t *game_state,
               deck_t *deck) {
    int nplayers = 2;
    int ncards = 6;

    hand_t *hands[2];
    hands[0] = new_hand(ncards);
    hands[1] = new_hand(ncards);
    hand_t *crib = new_hand(5);

    // Deal the hands.
    shuffle_deck(deck);
    int deck_offset = 0;
    for (int i = 0; i < ncards; i++) {
        hand_append(hands[0], deck->cards[deck_offset++]);
        hand_append(hands[1], deck->cards[deck_offset++]);
    }
    assert(deck->ncards - deck_offset == 40);
    assert(deck_offset == ncards * nplayers);

    playername_t *pname = game_state->player_name;

    // Discard cards using configured strategies. Have to sort first because
    // that's part of the contract with discard strategy functions.
    sort_cards(hands[0]->ncards, hands[0]->cards);
    log_cards(LOG_DEBUG,
              "hands[0] after dealing",
              hands[0]->ncards,
              hands[0]->cards);
    game_state->strategy[pname[0]].discard_func(hands[0], crib);
    log_cards(LOG_DEBUG,
              "hands[0] after discard",
              hands[0]->ncards,
              hands[0]->cards);

    sort_cards(hands[1]->ncards, hands[1]->cards);
    log_cards(LOG_DEBUG,
              "hands[1] after dealing",
              hands[1]->ncards,
              hands[1]->cards);
    game_state->strategy[pname[1]].discard_func(hands[1], crib);
    log_cards(LOG_DEBUG,
              "hands[1] after discard",
              hands[1]->ncards,
              hands[1]->cards);

    log_cards(LOG_DEBUG,
              "crib after discard    ",
              crib->ncards,
              crib->cards);

    // Turn up the starter card.
    int starter_idx = rand() % (deck->ncards - deck_offset);
    starter_idx += deck_offset;
    card_t starter = deck->cards[starter_idx];
    char buf[5];
    log_debug("starter: deck[%d] = %s", starter_idx, card_str(buf, starter));

    // Evaluate the results (including pegging).
    bool done = evaluate_hands(game_state,
                               nplayers,
                               hands,
                               crib,
                               starter);

    free(hands[0]);
    free(hands[1]);
    free(crib);

    return done;
}

char play_game(deck_t *deck) {
    gamestate_t game_state = gamestate_init();

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

    game_state.strategy[PLAYER_A] = strategy_a;
    game_state.strategy[PLAYER_B] = strategy_b;

    bool done = false;
    int num_hands = 0;
    //char *player_map;  // map player number (0 or 1) to name (a or b)
    char winner_name = 0;
    while (!done) {
        // Swap players: PLAYER_A becomes PLAYER_B and vice-versa.
        game_state.player_name[0] ^= 1;
        game_state.player_name[1] ^= 1;

        done = play_hand(&game_state, deck);
        num_hands++;
        stringbuilder_t winner_sb;
        sb_init(&winner_sb, 20);
        if (done) {
            assert(game_state.winner == 0 || game_state.winner == 1);
            winner_name = playername_as_char(game_state.winner);
            assert(winner_name == 'a' || winner_name == 'b');
            sb_printf(&winner_sb,
                      "winner=%c",
                      winner_name);
        }
        else {
            sb_append(&winner_sb, "no winner yet");
        }

        log_info("after %d hand(s): scores={a: %d, b: %d}, %s",
                 num_hands,
                 game_state.score[PLAYER_A],
                 game_state.score[PLAYER_B],
                 sb_as_string(&winner_sb));
        sb_close(&winner_sb);
    }
    return winner_name;
}
