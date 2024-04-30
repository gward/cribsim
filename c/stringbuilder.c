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

bool sb_append(stringbuilder_t sb, char *s) {
    size_t len = strlen(s);
    if (sb->count + len > sb->cap) {
        size_t old_cap = sb->cap;
        while (sb->cap < sb->count + len) {
            sb->cap *= LOAD_FACTOR;
        }

        char *new_mem = realloc(sb->mem, sb->cap);
        if (!new_mem) {
            return false;
        }
        memset(new_mem + old_cap, 0, old_cap);
        sb->mem = new_mem;
    }
    char *dst = stpncpy(sb->mem + sb->count, s, sb->cap - sb->count);
    assert(dst <= sb->mem + sb->cap);
    sb->count += len;
    return true;
}

bool sb_append_char(stringbuilder_t sb, char c) {
    if (sb->count + 1 > sb->cap) {
        size_t old_cap = sb->cap;
        sb->cap *= LOAD_FACTOR;
        char *new_mem = realloc(sb->mem, sb->cap);
        if (!new_mem) {
            return false;
        }
        memset(new_mem + old_cap, 0, old_cap);
        sb->mem = new_mem;
    }
    sb->mem[sb->count++] = c;
    return true;
}

char *sb_as_string(stringbuilder_t sb) {
    return sb->mem;
}

void sb_free(stringbuilder_t sb) {
    free(sb->mem);
    free(sb);
}
