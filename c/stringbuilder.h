// taken from https://codereview.stackexchange.com/questions/155286/stringbuilder-in-c

#ifndef _STRINGBUILDER_H
#define _STRINGBUILDER_H

#include <stddef.h>
#include <stdbool.h>

struct stringbuilder_s;
typedef struct stringbuilder_s *stringbuilder_t;
stringbuilder_t sb_new(size_t);
bool sb_append(stringbuilder_t to, char *s);
bool sb_append_char(stringbuilder_t, char);
char *sb_as_string(stringbuilder_t);
void sb_free(stringbuilder_t);

#endif
