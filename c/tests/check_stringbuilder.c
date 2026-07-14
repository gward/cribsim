#include <stdlib.h>
#include <string.h>

#include <check.h>

#include "../stringbuilder.h"

/* test case: stringbuilder */

static void assert_stringbuilder(stringbuilder_t *sb, char *expect_str, size_t expect_cap) {
    size_t expect_len = strlen(expect_str);
    ck_assert_str_eq(sb_as_string(sb), expect_str);
    ck_assert_int_eq(strlen(sb->mem), expect_len);
    ck_assert_int_eq((int) sb->mem[0], (int) expect_str[0]);
    ck_assert_int_eq((int) sb->mem[expect_len - 1], (int) expect_str[expect_len - 1]);
    ck_assert_int_eq((int) sb->mem[expect_len], 0);
    ck_assert_int_eq(sb->len, expect_len);
    ck_assert_int_eq(sb->cap, expect_cap);
}

START_TEST(test_stringbuilder_basics) {
    stringbuilder_t sb;
    sb_init(&sb, 1);

    ck_assert_int_eq(sb.len, 0);
    ck_assert_int_eq(sb.cap, 1);
    ck_assert_str_eq(sb_as_string(&sb), "");

    // A 31-byte string fits comfortable, as long as we double the capacity
    // (buffer size) several times.
    sb_append(&sb, "this is a moderately long strin");
    assert_stringbuilder(&sb, "this is a moderately long strin", 32);

    // Add one more byte and we have to double the capacity again.
    sb_append_char(&sb, 'g');
    assert_stringbuilder(&sb, "this is a moderately long string", 64);

    sb_close(&sb);
    sb_init(&sb, 1);

    // Same thing, but as a single string append.
    sb_append(&sb, "this is a moderately long string");
    assert_stringbuilder(&sb, "this is a moderately long string", 64);
}
END_TEST

START_TEST(test_stringbuilder_printf) {
    stringbuilder_t sb;
    sb_init(&sb, 8);

    sb_printf(&sb, "hello %s, my name is %s and I am %.1f years old", "world", "bob", 37.59);
    assert_stringbuilder(&sb, "hello world, my name is bob and I am 37.6 years old", 64);
}

START_TEST(test_stringbuilder_append_int) {
    stringbuilder_t sb;
    sb_init(&sb, 20);

    // Exercise sb_append_int() without having to expand the buffer.
    sb_append_int(&sb, 234254);
    sb_append_char(&sb, ',');
    sb_append_int(&sb, -549202);
    assert_stringbuilder(&sb, "234254,-549202", 20);

    // Same, but this time the buffer has to grow.
    sb_close(&sb);
    sb_init(&sb, 1);
    sb_append_int(&sb, -234254);
    assert_stringbuilder(&sb, "-234254", 8);
    sb_append(&sb, " , ");
    assert_stringbuilder(&sb, "-234254 , ", 16);
    sb_append_int(&sb, 938682);
    assert_stringbuilder(&sb, "-234254 , 938682", 32);
}
END_TEST

START_TEST(test_stringbuilder_zero_cap) {
    // sb_fit_buffer()'s growth loop is `while (sb->cap <= new_len) sb->cap
    // *= LOAD_FACTOR;`. If cap starts at 0, 0 * LOAD_FACTOR is still 0, so
    // the loop never terminates. sb_init(sb, 0) leaves cap == 0 (calloc(0,
    // ...) returns a non-NULL pointer on this platform, so sb_init does not
    // fail), so any append against it should hang instead of growing.
    stringbuilder_t sb;
    sb_init(&sb, 0);
    ck_assert_int_eq(sb.cap, 0);

    sb_append(&sb, "hello");    // must not hang

    sb_close(&sb);
}
END_TEST

START_TEST(test_stringbuilder_memset) {
    // sb_fit_buffer() uses memset() after realloc() to ensure that
    // the unused part of its buffer is all zeroes. Make sure this
    // works correctly. Force more than one doubling because this used
    // to have a bug: it worked fine with one doubling, but did not
    // memset() enough bytes after multiple doublings.
    char *mem = malloc(1024);
    memset(mem, 0xAA, 1024);
    stringbuilder_t sb = {.mem = mem, .len = 0, .cap = 4};

    // Force cap to grow 4 -> 8 -> 16 -> 32 in one sb_fit_buffer() call.
    sb_printf(&sb, "%s", "AAAAAAAAAAAAAAAAAAAA");    // 20 chars
    ck_assert_int_eq(sb.cap, 32);
    ck_assert_int_eq(sb.len, 20);

    // Everything past the written string + NUL, up to cap, must be zero.
    for (size_t i = sb.len + 1; i < sb.cap; i++) {
        ck_assert_msg(sb.mem[i] == 0,
                      "sb.mem[%zu] = 0x%02hhx, expected 0 (uninitialized capacity leaked)",
                      i,
                      (unsigned char) sb.mem[i]);
    }

    sb_close(&sb);
}
END_TEST

void add_stringbuilder_tests(Suite *suite) {
    TCase *tc_stringbuilder = tcase_create("stringbuilder");

    tcase_add_test(tc_stringbuilder, test_stringbuilder_basics);
    tcase_add_test(tc_stringbuilder, test_stringbuilder_printf);
    tcase_add_test(tc_stringbuilder, test_stringbuilder_append_int);
    tcase_add_test(tc_stringbuilder, test_stringbuilder_zero_cap);
    tcase_add_test(tc_stringbuilder, test_stringbuilder_memset);
    suite_add_tcase(suite, tc_stringbuilder);
}
