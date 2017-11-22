/*
 Copyright 2014-2017 Thumbtack, Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/


#include <check.h>
#include <stdlib.h>
#include "../strconst.h"
#include "../custom_check.h"
#include "../config_parser.h"
#include "../run_checks.h"


START_TEST(test_sanitize_str)
{
#define N_SANITIZE_STR_TESTS  4

    char empty_line[N_SANITIZE_STR_TESTS][100] = {"", "# to ignore",
                                                  " # to ignore  ",
                                                  " # to ignore  \n"};
    char foo_line[N_SANITIZE_STR_TESTS][100] = {"foo = 1", "  foo = 1  ",
                                                " foo = 1 # yada, yada  ",
                                                " foo = 1 # yada, yada  \n"};
    int i;

    /* test config lines cleanup */
    for (i = 0; i < N_SANITIZE_STR_TESTS; i++) {
        sanitize_str(empty_line[i]);
        ck_assert_str_eq(empty_line[i], "");
        sanitize_str(foo_line[i]);
        ck_assert_str_eq(foo_line[i], "foo = 1");
    }
}
END_TEST

START_TEST(test_configparser)
{
    config_t config;

    /* make sure it fails when the config file doesn't exist */
    config = config_parse("doesnt_exist");
    ck_assert(config == NULL);

    /* this being hardcoded doesn't feel right, but there are more
     * important things to do right now */
    /* doesn't fail with an existing, statically correct, file */
    config = config_parse("tests/pgdoctor.cfg");
    ck_assert(config != NULL);
    ck_assert_int_eq(CFG_HTTP_PORT(config), 8071);
    ck_assert_str_eq(CFG_SYSLOG_FACILITY(config), "local7");
    ck_assert_str_eq(CFG_PG_HOST(config), "localhost");
    ck_assert_int_eq(CFG_PG_PORT(config), 5432);
    ck_assert_str_eq(CFG_PG_USER(config), "postgres");
    ck_assert_str_eq(CFG_PG_PASSWORD(config), "");
    ck_assert_str_eq(CFG_PG_DATABASE(config), "postgres");
    ck_assert_int_eq(CFG_PG_TIMEOUT(config), 3);

    /* make sure to cleanup */
    config_destroy(config);
}
END_TEST

START_TEST(test_custom_check)
{
    custom_check_t check;
    char *q1 = "select count(*) from usr_users", *q2 = "select 1";
    char *v1 = "4", *v2 = "5";
    char *op1 = "", *op2 = "=";

    /* initialize a data struture and make sure it works */
    check = custom_check_create(q1, op1, v1);
    ck_assert_str_eq(CUSTOM_CHECK_QUERY(check), q1);
    ck_assert_str_eq(CUSTOM_CHECK_RESULT(check), v1);
    ck_assert_str_eq(CUSTOM_CHECK_OPERATOR(check), op1);

    /* change the data type and make sure it succedeed */
    custom_check_set_query(check, q2);
    custom_check_set_result(check, v2);
    custom_check_set_operator(check, op2);
    ck_assert_str_eq(CUSTOM_CHECK_QUERY(check), q2);
    ck_assert_str_eq(CUSTOM_CHECK_RESULT(check), v2);
    ck_assert_str_eq(CUSTOM_CHECK_OPERATOR(check), op2);

    /* make sure to cleanup */
    custom_check_destroy(check);
}
END_TEST

START_TEST(test_run_checks)
{
    char result[MAX_STR];
    config_t config;

    config = config_parse("tests/pgdoctor.cfg");

    ck_assert_int_eq(run_health_checks(config, result, MAX_STR), 1);

    /* make sure to cleanup */
    config_destroy(config);
}
END_TEST

START_TEST(test_failed_run_checks)
{
    char result[MAX_STR];
    config_t config;

    config = config_parse("tests/pgdoctor_fails.cfg");

    ck_assert_int_eq(run_health_checks(config, result, MAX_STR), 0);

    /* make sure to cleanup */
    config_destroy(config);
}
END_TEST

int main(void)
{
    int nf;

    Suite *s = suite_create("pgDoctor");
    TCase *tc_config_parser = tcase_create("config_parser");
    TCase *tc_custom_checks = tcase_create("Core");
    SRunner *sr = srunner_create(s);

    suite_add_tcase(s, tc_config_parser);
    tcase_add_test(tc_config_parser, test_sanitize_str);
    tcase_add_test(tc_config_parser, test_configparser);

    suite_add_tcase(s, tc_custom_checks);
    tcase_add_test(tc_custom_checks, test_custom_check);
    tcase_add_test(tc_custom_checks, test_run_checks);
    tcase_add_test(tc_custom_checks, test_failed_run_checks);

    srunner_run_all(sr, CK_NORMAL);
    nf = srunner_ntests_failed(sr);

    srunner_free(sr);

    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
