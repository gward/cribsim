// taken from https://codereview.stackexchange.com/questions/155286/stringbuilder-in-c

#define _POSIX_C_SOURCE 200809L    // for stpncpy()

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "stringbuilder.h"

bool sb_init(stringbuilder_t *sb, size_t init_cap) {
    sb->cap = init_cap;
    sb->len = 0;
    sb->mem = calloc(init_cap, sizeof(char));
    if (!sb->mem) {
        sb->cap = 0;
        return false;
    }
    return true;
}

void sb_close(stringbuilder_t *sb)
{
    if (sb->mem != NULL) {
        free(sb->mem);
        sb->mem = NULL;
    }
    sb->cap = 0;
    sb->len = 0;
}

stringbuilder_t *sb_new(size_t init_cap) {
    stringbuilder_t *ret = malloc(sizeof(stringbuilder_t));
    if (!ret) {
        return NULL;
    }
    bool ok = sb_init(ret, init_cap);
    if (!ok) {
        free(ret);
        return NULL;
    }
    return ret;
}

void sb_free(stringbuilder_t *sb) {
    sb_close(sb);
    free(sb);
}

#define LOAD_FACTOR 2

bool sb_append(stringbuilder_t *sb, char *s) {
    size_t len = strlen(s);
    size_t old_cap = sb->cap;
    while (sb->cap <= sb->len + len) {
        sb->cap *= LOAD_FACTOR;
    }
    if (sb->cap != old_cap) {
        char *new_mem = realloc(sb->mem, sb->cap);
        if (!new_mem) {
            return false;
        }
        memset(new_mem + old_cap, 0, old_cap);
        sb->mem = new_mem;
    }

    char *dst = stpncpy(sb->mem + sb->len, s, sb->cap - sb->len);
    assert(dst <= sb->mem + sb->cap);
    sb->len += len;
    return true;
}

bool sb_append_char(stringbuilder_t *sb, char c) {
    if (sb->cap <= sb->len + 1) {
        size_t old_cap = sb->cap;
        sb->cap *= LOAD_FACTOR;
        char *new_mem = realloc(sb->mem, sb->cap);
        if (!new_mem) {
            return false;
        }
        memset(new_mem + old_cap, 0, old_cap);
        sb->mem = new_mem;
    }
    sb->mem[sb->len++] = c;
    return true;
}

char *sb_as_string(stringbuilder_t *sb) {
    return sb->mem;
}
