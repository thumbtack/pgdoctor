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


#include "run_checks.h"
#include <string.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "logger.h"
#include "strconst.h"



/* write the error string to `result` and terminate the connection
 * gracefully */
static void pg_fail(PGconn *pg_conn, char *result, size_t size)
{
    char *errormsg = PQerrorMessage(pg_conn);

    /* if there one, save the error message from PG to `result` and
     * gracefully terminate the DB connection */
    if (errormsg)
        snprintf(result, size, "%s", errormsg);
}

static int run_custom_check(PGconn *pg_conn, custom_check_t check,
                            char *result, size_t size)
{
    PGresult *pg_result;
    char *query_result;
    int success = 1;

    pg_result = PQexec(pg_conn, CUSTOM_CHECK_QUERY(check));

    /* without a result set there's no point on proceeding */
    if (PQresultStatus(pg_result) != PGRES_TUPLES_OK) {
                pg_fail(pg_conn, result, size);
                PQclear(pg_result);
        return 0;
    }

    /* SELECT command that happens to retrieve zero rows still shows
       PGRES_TUPLES_OK */
    if (PQntuples(pg_result) != 1) {
        pg_fail(pg_conn, result, size);
        PQclear(pg_result);
        return 0;
    }

    /* we only expect one field with one result */
    query_result = PQgetvalue(pg_result, 0, 0);

    /* an empty string means that there is nothing to compare to, so
     * the check fails */
    if (strlen(query_result) == 0) {
                snprintf(result, size, STR_QUERY_FAILED_FMT, CUSTOM_CHECK_QUERY(check));
                PQclear(pg_result);
                return 0;
    }

    /* compare the query's result with the expected value */
    /* = compares as a string */
    if (strcmp(CUSTOM_CHECK_OPERATOR(check), "=") == 0)
                success = (strcmp(query_result, CUSTOM_CHECK_RESULT(check)) == 0);
    /* < and > compare as floating point values */
    if (strcmp(CUSTOM_CHECK_OPERATOR(check), "<") == 0)
                success = (atof(query_result) < atof(CUSTOM_CHECK_RESULT(check)));
    if (strcmp(CUSTOM_CHECK_OPERATOR(check), ">") == 0)
        success = (atof(query_result) > atof(CUSTOM_CHECK_RESULT(check)));

    /* make sure the error message is written to the result string */
    if (! success)
        snprintf(result, size, STR_HEALTH_CHECK_ERROR_DETAIL_FMT,
                 CUSTOM_CHECK_QUERY(check),
                 CUSTOM_CHECK_OPERATOR(check),
                 CUSTOM_CHECK_RESULT(check),
                 query_result);

    PQclear(pg_result);
    return success;
}

static int run_all_checks(PGconn *pg_conn, checks_list_t checks_list,
                                                  char *result, size_t size)
{
    int success = 1;
    custom_check_t check;

    while (checks_list) {
        check = CHECKS_LIST_CHECK(checks_list);
        if (run_custom_check(pg_conn, check, result, size)) {
            logger_write(LOG_INFO, STR_HEALTH_CHECK_SUCCESS_FMT,
                         CUSTOM_CHECK_QUERY(check),
                         CUSTOM_CHECK_OPERATOR(check),
                         CUSTOM_CHECK_RESULT(check));
        } else {
            success = 0;
            logger_write(LOG_ERR, STR_HEALTH_CHECK_ERROR_FMT,
                         CUSTOM_CHECK_QUERY(check),
                         CUSTOM_CHECK_OPERATOR(check),
                         CUSTOM_CHECK_RESULT(check));
        }
        checks_list = CHECKS_LIST_NEXT(checks_list);
    }

    return success;
}

/* top level health check: connects to the DB and runs all checks */
/* returns 0 on pg_failure and 1 on sucess */
/* a string with the description of the error (or "OK") is copied to
 * `result` */
extern int run_health_checks(config_t config, char *result, size_t size)
{
    /* connection strings: format and actual string to be created */
    char pg_conn_str[MAX_STR];
    PGconn *pg_conn;

    /* create the connection string from the set of parameters */
    snprintf(pg_conn_str,
             sizeof(pg_conn_str),
             STR_PG_CONN_INFO_FMT,
             CFG_PG_HOST(config),
             CFG_PG_PORT(config),
             CFG_PG_DATABASE(config),
             CFG_PG_USER(config),
             CFG_PG_PASSWORD(config),
             CFG_PG_TIMEOUT(config));

    /* check successful connection or return error right now */
    pg_conn = PQconnectdb(pg_conn_str);
    if (PQstatus(pg_conn) != CONNECTION_OK) {
        logger_write(LOG_CRIT, STR_DB_CONNECTION_ERROR);
        pg_fail(pg_conn, result, size);
        PQfinish(pg_conn);
        return 0;
    }

    /* run the custom checks */
    if (! run_all_checks(pg_conn, CFG_CUSTOM_CHECKS(config), result, size)) {
        PQfinish(pg_conn);
        return 0;
    }

    snprintf(result, size, "%s", STR_ALL_CHECKS_SUCCESSFUL);
    PQfinish(pg_conn);
    return 1;
}
