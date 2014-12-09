#include <string.h>
#include <stdlib.h>
#include <libpq-fe.h>
#include "config_parser.h"
#include "run_checks.h"
#include "strconst.h"



/* write the error string to `result` and terminate the connection
 * gracefully */
static void pg_fail(PGconn *pg_conn, char *result)
{
    char *errormsg = PQerrorMessage(pg_conn);

    /* if there one, save the error message from PG to `result` and
     * gracefully terminate the DB connection */
    if (errormsg)
	strcpy(result, errormsg);
    PQfinish(pg_conn);
}

static int check_replication_lag(PGconn *pg_conn, config_t config, char *result)
{
    PGresult *pg_result;
    char *lag;

    /* not checking replication lag == passes the check */
    if (CFG_REPLICATION_LAG(config) < 0)
	return 1;

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
     * the check was enabled so it should pg_fail */
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

/* top level health check: connects to the DB and runs all checks */
/* returns 0 on pg_failure and 1 on sucess */
/* a string with the description of the error (or "OK") is copied to
 * `result` */
extern int run_health_checks(config_t config, char *result)
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
    	pg_fail(pg_conn, result);
    	return 0;
    }

    /* check replication lag */
    if (! check_replication_lag(pg_conn, config, result))
    	return 0;

    /* run the custom checks */

    /* if we made it this far no check failed; kill the database
     * connection, write something nice to the result string, and
     * return */
    PQfinish(pg_conn);
    strcpy(result, STR_OK);
    return 1;
}
