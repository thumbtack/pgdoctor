#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "health_check.h"


extern health_check_t health_check_create(char *query, char *expected_result, char *operator)
{
    health_check_t check = malloc(sizeof(health_check_t));


    if (check) {
	HEALTH_CHECK_QUERY(check) = malloc(strlen(query)+1);
	HEALTH_CHECK_RESULT(check) = malloc(strlen(expected_result)+1);
	HEALTH_CHECK_OPERATOR(check) = malloc(strlen(operator)+1);
	if (HEALTH_CHECK_QUERY(check) &&
	    HEALTH_CHECK_RESULT(check) &&
	    HEALTH_CHECK_OPERATOR(check)) {
	    strcpy(HEALTH_CHECK_QUERY(check), query);
	    strcpy(HEALTH_CHECK_RESULT(check), expected_result);
	    strcpy(HEALTH_CHECK_OPERATOR(check), operator);
	}
    }
    else {
	/* log it */
    }

    return check;
}

static void set_check_parameter(char **param, char *value)
{
    *param = realloc(*param, strlen(value)+1);
    if (*param) {
	strcpy(*param, value);
    }

}

extern void health_check_set_query(health_check_t check, char *query)
{
    if (check)
	set_check_parameter(&HEALTH_CHECK_QUERY(check), query);
}

extern void health_check_set_result(health_check_t check, char *expected_result)
{
    if (check)
	set_check_parameter(&HEALTH_CHECK_RESULT(check), expected_result);
}

extern void health_check_set_operator(health_check_t check, char *operator)
{
    if (check)
	set_check_parameter(&HEALTH_CHECK_OPERATOR(check), operator);
}

extern void health_check_destroy(health_check_t check)
{
    if (check) {
	if (HEALTH_CHECK_QUERY(check))
	    free(HEALTH_CHECK_QUERY(check));
	if (HEALTH_CHECK_RESULT(check))
	    free(HEALTH_CHECK_RESULT(check));
	if (HEALTH_CHECK_OPERATOR(check))
	    free(HEALTH_CHECK_OPERATOR(check));
    }
    free(check);
}
