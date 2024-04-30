// taken from https://codereview.stackexchange.com/questions/155286/stringbuilder-in-c

#define _POSIX_C_SOURCE 200809L    // for stpncpy()

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "stringbuilder.h"

struct stringbuilder_s {
    char *mem;
    size_t count;
    size_t cap;
};

typedef struct stringbuilder_s *stringbuilder_t;

stringbuilder_t sb_new(size_t init_cap) {
    stringbuilder_t ret = malloc(sizeof(struct stringbuilder_s));
    if (!ret) {
        return NULL;
    }
    ret->mem = calloc(init_cap, sizeof(char));
    if (!ret->mem) {
        free(ret);
        return NULL;
    }
    ret->cap = init_cap;
    ret->count = 0;
    return ret;
}

#define LOAD_FACTOR 2

bool sb_append(stringbuilder_t to, char *s) {
    size_t len = strlen(s);
    if (to->count + len > to->cap) {
        size_t old_cap = to->cap;
        while (to->cap < to->count + len) {
            to->cap *= LOAD_FACTOR;
        }

        char *new_mem = realloc(to->mem, to->cap);
        if (!new_mem) {
            return false;
        }
        memset(new_mem + old_cap, 0, old_cap);
        to->mem = new_mem;
    }
    char *dst = stpncpy(to->mem + to->count, s, to->cap - to->count);
    assert(dst <= to->mem + to->cap);
    to->count += len;
    return true;
}

bool sb_append_char(stringbuilder_t to, char c) {
    to->mem[to->count] = c;
    ++to->count;
    if (to->count == to->cap) {
        char *new_mem = realloc(to->mem, to->cap * LOAD_FACTOR);
        if (!new_mem) {
            return false;
        }
        memset(new_mem + to->cap, 0, to->cap);
        to->mem = new_mem;
        to->cap *= LOAD_FACTOR;
    }
    return true;
}

char *sb_as_string(stringbuilder_t sb) {
    return sb->mem;
}

void sb_free(stringbuilder_t sb) {
    free(sb->mem);
    free(sb);
}
