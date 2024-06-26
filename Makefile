CFLAGS = -g -Wall -Werror -std=c99
LDFLAGS = -g

SRC = $(wildcard c/*.c)
#$(info SRC=$(SRC))
OBJ = $(patsubst %.c,build/%.o,$(notdir $(SRC)))
#$(info OBJ=$(OBJ))

TESTOBJ = $(filter-out build/cribsim.o,$(OBJ))
$(info TESTOBJ=$(TESTOBJ))

all: build/cribsim

build/%.o: c/%.c c/*.h
	mkdir -p build && $(CC) $(CFLAGS) -c -o $@ $<

build/cribsim: $(OBJ)
	mkdir -p build
	$(CC) $(LDFLAGS) -o $@ $^

build/check_cribsim: c/tests/check_cribsim.c $(TESTOBJ)
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^ -lcheck -lsubunit -lm

check: build/check_cribsim
	$<

grind: build/cribsim
	valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all $<
