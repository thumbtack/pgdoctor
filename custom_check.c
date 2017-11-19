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
