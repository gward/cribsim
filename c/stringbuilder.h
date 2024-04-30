// taken from https://codereview.stackexchange.com/questions/155286/stringbuilder-in-c

#ifndef _STRINGBUILDER_H
#define _STRINGBUILDER_H

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    char *mem;
    size_t len;
    size_t cap;
} stringbuilder_t;

bool sb_init(stringbuilder_t *, size_t init_cap);
void sb_close(stringbuilder_t *);

stringbuilder_t *sb_new(size_t);
void sb_free(stringbuilder_t *);

bool sb_append(stringbuilder_t *, char *);
bool sb_printf(stringbuilder_t *sb, const char *fmt, ...);
bool sb_append_int(stringbuilder_t *, int);
bool sb_append_char(stringbuilder_t *, char);
char *sb_as_string(stringbuilder_t *);

#endif
