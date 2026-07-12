# cribsim

A cribbage simulator written in C. It plays many games of two-player cribbage
between two computer players, each following a configurable strategy, in order
to compare strategies and understand how well they perform.


## What it does

`cribsim` simulates complete games of two-player cribbage from deal through
pegging and hand/crib scoring. After playing a batch of games it reports how
many each player won.

Different strategies can be selected at runtime using command-line options.

## Building

### Prerequisites

* C99-capable compiler
* standard POSIX libraries
* (optional) [Check](https://libcheck.github.io/check/) unit-testing library
  (`libcheck` and `libsubunit`)
* (optional) valgrind

Compile to `build/cribsim`:

```
make
```

Compile and run unit tests (`build/check_cribsim`):
```
make check
```

Run under Valgrind to check for leaks and other memory errors:

```
make grind
```

## Running

```
./build/cribsim
```

By default, the program plays 100 games and writes one log line per
hand to stderr, followed by a final summary:

```
INFO  play.c:714: after 1 hand(s): scores={a: 14, b: 7}, no winner yet
INFO  play.c:714: after 2 hand(s): scores={a: 21, b: 19}, no winner yet
...
INFO  play.c:714: after 17 hand(s): scores={a: 121, b: 93}, winner=a
INFO  cribsim.c:29: player a: 54 wins, player b: 46 wins
```

Use options `--strategy-a` and `--strategy-b` to specify different
pegging and discard strategies for the two players:

```
./build/cribsim --strategy-a low,random --strategy-a high,simple
```

Cards are formatted with Unicode suit symbols (♣ ♦ ♥ ♠) and rank characters
`A234567890JQK`.

The RNG is seeded from the current time and process ID, so results vary
between runs.

## Strategies

Two strategy axes are independently configurable: **discarding** (which two
cards to put in the crib) and **pegging** (which card to play during the
count).

### Discard strategies

| Name      | Description                                                                                                                                   |
|-----------|-----------------------------------------------------------------------------------------------------------------------------------------------|
| `simple`  | Enumerates all 15 ways to choose 4 cards from the 6 dealt. Keeps the 4 with the highest static score (no starter card). Ties go to the first combination found. |
| `random`  | Discards two randomly chosen cards. Useful as a baseline.                                                                                     |

`discard_simple` ignores the starter card, expected value over possible
starters, whether the crib belongs to you or your opponent, and secondary
tiebreaking criteria.

### Pegging strategies

| Name   | Description                                                  |
|--------|--------------------------------------------------------------|
| `low`  | Always plays the lowest card that keeps the count ≤ 31.      |
| `high` | Always plays the highest card that keeps the count ≤ 31.     |

Both strategies are purely reactive — they have no knowledge of the
opponent's hand and do no look-ahead. Neither tries to make 15s or 31s
deliberately.

## What you can change (without major surgery)

- **Log verbosity** — change the `log_set_level()` call in `main()`. Use
  `LOG_DEBUG` to see dealt/discarded cards and scoring breakdowns per hand,
  or `LOG_TRACE` for every pegging move and combo evaluation. `LOG_WARN`
  silences the per-hand lines, leaving only the final summary.
- **New strategy** — implement any function with the right signature and plug
  it in:
  - Discard: `void my_discard(hand_t *hand, hand_t *crib)`
  - Pegging: `int my_peg(peg_state_t *peg, int player, int other)`

## Known limitations

- **No structured output.** Results go to stderr in human-readable log format
  only; there is no CSV, JSON, or other format suitable for further analysis.
- **Strategies are naive.** Neither discard nor pegging strategy does any
  probability calculation or look-ahead.
- **Flush scoring doesn't distinguish hand from crib.** Standard cribbage
  requires all five cards to match for a crib flush, but the code applies the
  same rule (4 or 5 cards) to both.

## Code overview

| File                          | Purpose                                              |
|-------------------------------|------------------------------------------------------|
| `c/cribsim.c`                 | `main()`: seeds RNG, runs games, prints summary      |
| `c/play.c` / `play.h`         | Game loop, hand play, pegging, strategies            |
| `c/score.c` / `score.h`       | Hand scoring: 15s, pairs, runs, flush, nobs          |
| `c/cards.c` / `cards.h`       | Card and deck types, shuffling, formatting           |
| `c/twiddle.c` / `twiddle.h`   | Combination iterator (Chase's Algorithm 382)         |
| `c/stringbuilder.c` / `.h`    | Growable string buffer used for log messages         |
| `c/log.c` / `log.h`           | `rxi/log.c` logging library (git submodule)          |
| `c/tests/check_cribsim.c`     | Unit tests (Check framework)                         |
