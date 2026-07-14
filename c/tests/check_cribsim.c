#include <stdlib.h>

#include <check.h>

void add_stringbuilder_tests(Suite *suite);
void add_cards_tests(Suite *suite);
void add_score_tests(Suite *suite);
void add_play_tests(Suite *suite);

Suite *cribsum_suite(void) {
    Suite *suite = suite_create("cribsim");

    add_stringbuilder_tests(suite);
    add_cards_tests(suite);
    add_score_tests(suite);
    add_play_tests(suite);

    return suite;
}

int main(void) {
    Suite *suite = cribsum_suite();
    SRunner *runner = srunner_create(suite);

    srunner_run_all(runner, CK_NORMAL);
    int num_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
