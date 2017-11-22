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


#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L /* for strdup */

#include "custom_check.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "logger.h"

extern custom_check_t custom_check_create(char *query, char *operator,
                                          char *expected_result)
{
    custom_check_t check = malloc(sizeof(struct custom_check));


    if (check) {
        CUSTOM_CHECK_QUERY(check) = strdup(query);
        CUSTOM_CHECK_RESULT(check) = strdup(expected_result);
        CUSTOM_CHECK_OPERATOR(check) = strdup(operator);
        if (! (CUSTOM_CHECK_QUERY(check) &&
               CUSTOM_CHECK_RESULT(check) &&
               CUSTOM_CHECK_OPERATOR(check))) {
            logger_write(LOG_CRIT, "%m");
        }
    } else {
        logger_write(LOG_CRIT, "%m");
    }

    return check;
}

static void set_check_parameter(char **param, const char *value)
{
    *param = realloc(*param, strlen(value)+1);
    if (*param) {
        strcpy(*param, value);
    }
}

extern void custom_check_set_query(custom_check_t check, char *query)
{
    if (check) {
        set_check_parameter(&CUSTOM_CHECK_QUERY(check), query);
    }
}

extern void custom_check_set_result(custom_check_t check, char *expected_result)
{
    if (check) {
        set_check_parameter(&CUSTOM_CHECK_RESULT(check), expected_result);
    }
}

extern void custom_check_set_operator(custom_check_t check, char *operator)
{
    if (check) {
        set_check_parameter(&CUSTOM_CHECK_OPERATOR(check), operator);
    }
}

extern void custom_check_destroy(custom_check_t check)
{
    if (check) {
        if (CUSTOM_CHECK_QUERY(check)) {
            free(CUSTOM_CHECK_QUERY(check));
        }
        if (CUSTOM_CHECK_RESULT(check)) {
            free(CUSTOM_CHECK_RESULT(check));
        }
        if (CUSTOM_CHECK_OPERATOR(check)) {
            free(CUSTOM_CHECK_OPERATOR(check));
        }
        free(check);
    }
}
