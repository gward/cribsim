CFLAGS = -g -Wall -Werror -std=c99
LDFLAGS = -g

SRC = $(wildcard c/*.c)
#$(info SRC=$(SRC))
OBJ = $(patsubst %.c,build/%.o,$(notdir $(SRC)))
#$(info OBJ=$(OBJ))

# Vendored third-party code (c/log.c, c/log.h, c/rxilog/) is excluded:
# it has its own style and isn't ours to reformat.
FMT_SRC = $(filter-out c/log.c,$(SRC)) $(wildcard c/tests/*.c)
FMT_HDR = $(filter-out c/log.h,$(wildcard c/*.h))

TESTSRC = $(wildcard c/tests/*.c)
TESTOBJ = \
    $(filter-out build/cribsim.o,$(OBJ)) \
    $(patsubst c/tests/%.c,build/tests/%.o,$(TESTSRC))
$(info TESTOBJ=$(TESTOBJ))

all: build/cribsim

build/%.o: c/%.c c/*.h
	mkdir -p build && $(CC) $(CFLAGS) -c -o $@ $<

build/tests/%.o: c/tests/%.c c/*.h c/tests/*.h
	mkdir -p build/tests && $(CC) $(CFLAGS) -c -o $@ $<

build/cribsim: $(OBJ)
	mkdir -p build
	$(CC) $(LDFLAGS) -o $@ $^

build/check_cribsim: $(TESTOBJ)
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^ -lcheck -lsubunit -lm

check: build/check_cribsim
	$<

grind: build/cribsim
	valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all $<

format:
	clang-format -i $(FMT_SRC) $(FMT_HDR)
