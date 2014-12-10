#include <check.h>
#include <stdlib.h>
#include "../strconst.h"
#include "../custom_check.h"
#include "../config_parser.h"

START_TEST(test_check_type)
{
    custom_check_t check;
    char *q1 = "select count(*) from usr_users", *q2 = "select 1";
    char *v1 = "4", *v2 = "5";
    char *op1 = "", *op2 = "=";

    /* initialize a data struture and make sure it works */
    check = custom_check_create(q1, v1, op1);
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
    ck_assert_int_eq(CFG_REPLICATION_LAG(config), -1);

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
    tcase_add_test(tc1_1, test_check_type);
    tcase_add_test(tc1_1, test_configparser);

    srunner_run_all(sr, CK_NORMAL);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

}
