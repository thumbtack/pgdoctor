#include "run_checks.h"
#include <string.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "config_parser.h"
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

static int check_replication_lag(PGconn *pg_conn, config_t config, char *result)
{
    PGresult *pg_result;
    char *lag;

    pg_result = PQexec(pg_conn, STR_REPLICATION_LAG_QUERY);

    /* without a result set there's no point on proceeding */
    if (PQresultStatus(pg_result) != PGRES_TUPLES_OK) {
	PQclear(pg_result);
    	return 0;
    }

    /* we only expect one field with one result */
    lag = PQgetvalue(pg_result, 0, 0);

    /* an empty string means that there is no record of the last
     * transaction log replay, but if we got this far it means that
     * the check was enabled so it should fail */
    if (strlen(lag) == 0) {
	/* there is no error on the PG side, so lets just set `result`
	 * here */
	strcpy(result, STR_NO_REPLICATION_INFO);
	PQclear(pg_result);
	return 0;
    }

    /* at this point there is an integer for the replication lag; fail
     * iff it is greater than the specified limit */
    PQclear(pg_result);
    return atoi(lag) <= CFG_REPLICATION_LAG(config);
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

    /* we only expect one field with one result */
    query_result = PQgetvalue(pg_result, 0, 0);

    /* an empty string means that there is nothing to compare to, so
     * the check fails */
    if (strlen(query_result) == 0) {
	snprintf(result, size, STR_QUERY_FAILED, CUSTOM_CHECK_QUERY(check));
	PQclear(pg_result);
	return 0;
    }

    /* compare the query's result with the expected value */
    /* = compares as a string */
    if (strcmp(CUSTOM_CHECK_OPERATOR(check), "="))
	success = (strcmp(query_result, CUSTOM_CHECK_RESULT(check)) == 0);
    /* < and > compare as floating point values */
    if (strcmp(CUSTOM_CHECK_OPERATOR(check), "<") == 0)
	success = (atof(query_result) < atof(CUSTOM_CHECK_RESULT(check)));
    if (strcmp(CUSTOM_CHECK_OPERATOR(check), ">") == 0)
    	success = (atof(query_result) > atof(CUSTOM_CHECK_RESULT(check)));

    /* at this point there is an integer for the replication lag; fail
     * iff it is greater than the specified limit */
    PQclear(pg_result);
    return success;
}

static int run_all_checks(PGconn *pg_conn, checks_list_t checks_list,
			  char *result, size_t size)
{
    custom_check_t check;

    while (checks_list) {
	check = CHECKS_LIST_CHECK(checks_list);
	if (! run_custom_check(pg_conn, check, result, size)) {
	    logger_write(LOG_ERR, STR_RUN_CHECK_ERROR,
			 CUSTOM_CHECK_QUERY(check),
			 CUSTOM_CHECK_OPERATOR(check),
			 CUSTOM_CHECK_RESULT(check));
	    return 0;
	} else {
	    logger_write(LOG_INFO, STR_RUN_CHECK_SUCCESS,
			 CUSTOM_CHECK_QUERY(check),
			 CUSTOM_CHECK_OPERATOR(check),
			 CUSTOM_CHECK_RESULT(check));
	}
	checks_list = CHECKS_LIST_NEXT(checks_list);
    }

    return 1;
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
	     MAX_STR,
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

    /* check replication lag */
    if (CFG_REPLICATION_LAG(config) >= 0) {
	if (! check_replication_lag(pg_conn, config, result)) {
	    logger_write(LOG_ERR, STR_RUN_CHECK_ERROR, "replication lag", "", "");
	    PQfinish(pg_conn);
	    return 0;
	}
    }

    /* run the custom checks */
    if (! run_all_checks(pg_conn, CFG_CUSTOM_CHECKS(config), result, size)) {
	PQfinish(pg_conn);
	return 0;
    }

    strcpy(result, STR_OK);
    PQfinish(pg_conn);
    return 1;
}
