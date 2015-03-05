/* Copyright (c) 2014, 2015 Thumbtack */
/* All rights reserved. */

/* Redistribution and use in source and binary forms, with or without */
/* modification, are permitted provided that the following conditions are met: */

/* * Redistributions of source code must retain the above copyright notice, this */
/*   list of conditions and the following disclaimer. */

/* * Redistributions in binary form must reproduce the above copyright notice, */
/*   this list of conditions and the following disclaimer in the documentation */
/*   and/or other materials provided with the distribution. */

/* * Neither the name of pgdoctor nor the names of its */
/*   contributors may be used to endorse or promote products derived from */
/*   this software without specific prior written permission. */

/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" */
/* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE */
/* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE */
/* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL */
/* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR */
/* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER */
/* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */


#include <check.h>
#include <stdlib.h>
#include "../strconst.h"
#include "../custom_check.h"
#include "../config_parser.h"
#include "../run_checks.h"

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

START_TEST(test_configparser)
{
    config_t config;
    char empty_line[4][100] = {"", "# to ignore", " # to ignore  "};
    char foo_line[4][100] = {"foo = 1", "  foo = 1  ", " foo = 1 # yada, yada  "};
    int i;

    /* test config lines cleanup */
    for (i=0; i<3; i++) {
	sanitize_str(empty_line[i]);
	ck_assert_str_eq(empty_line[i], "");
	sanitize_str(foo_line[i]);
	ck_assert_str_eq(foo_line[i], "foo = 1");
    }

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
    config_cleanup(config);
}
END_TEST

START_TEST(test_run_checks)
{
    char result[MAX_STR];
    config_t config;

    config = config_parse("tests/pgdoctor.cfg");

    ck_assert_int_eq(run_health_checks(config, result, MAX_STR), 1);

    /* make sure to cleanup */
    config_cleanup(config);
}
END_TEST

START_TEST(test_failed_run_checks)
{
    char result[MAX_STR];
    config_t config;

    config = config_parse("tests/pgdoctor_fails.cfg");

    ck_assert_int_eq(run_health_checks(config, result, MAX_STR), 0);

    /* make sure to cleanup */
    config_cleanup(config);
}
END_TEST

int main(void)
{

    Suite *s1 = suite_create("Core");
    TCase *tc1_1 = tcase_create("Core");
    SRunner *sr = srunner_create(s1);
    int nf;

    suite_add_tcase(s1, tc1_1);
    tcase_add_test(tc1_1, test_custom_check);
    tcase_add_test(tc1_1, test_configparser);
    tcase_add_test(tc1_1, test_run_checks);
    tcase_add_test(tc1_1, test_failed_run_checks);

    srunner_run_all(sr, CK_NORMAL);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

}
