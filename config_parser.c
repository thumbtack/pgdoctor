#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <libpq-fe.h>
#include "config_parser.h"
#include "logger.h"
#include "strconst.h"


/* map strings from the configuration file to equivalent constants */
#define MK_CONFIG_LIST(f) \
    f(HTTP_PORT) f(SYSLOG_FACILITY) f(PG_HOST) f(PG_PORT) f(PG_USER) \
    f(PG_PASSWORD) f(PG_DATABASE) f(PG_CONNECTION_TIMEOUT) \
    f(PG_MAX_REPLICATION_LAG) f(HEALTH_CHECK)
#define N_CONFIG_PARAMS 10
#define MK_CONFIG_PAIR(X) {X, #X},
#define MK_CONFIG_ENUM(X) X,

typedef enum { MK_CONFIG_LIST(MK_CONFIG_ENUM) } config_option_t;

static struct {
    int param_const;
    char * param_str;
} glb_config_param[] = { MK_CONFIG_LIST(MK_CONFIG_PAIR) };


/* remove comments and whitespace (in-place) from a line in the
 * configuration file */
void sanitize_str(char *str)
{
    char *ignore_after, *start_from = str;
    int length;

    /* find the first occurrence of # to ignore everything that
     * follows */
    ignore_after = strchr(str, COMMENT_CHR);
    if (! ignore_after)
	ignore_after = str+strlen(str);
    /* move back 1 chr (either the comment mark or \0) */
    ignore_after--;
    /* ignore any whitespace before the comment */
    while (isspace(*ignore_after) && ignore_after >= str)
    	ignore_after--;

    /* ignore whitespace at the beginning of the string */
    while (isspace(*start_from) && start_from < ignore_after)
    	start_from++;

    /* size of the sanitized configuration str */
    length = ignore_after-start_from;

    /* move the useful bytes to the beginning of the string */
    memmove(str, start_from, length+1);
    /* make sure to terminate the string */
    str[length+1] = '\0';
}

/* parse a (sanitized) line, extract and load the parameter,
 * respective value, and possible relational operator (in the case of
 * SQL checks) into `param`/`value`/`op`; `op` will be set to an
 * empty string */
void parse_line(char *param, char *value, char *op, const char *line)
{
    /* char *assign = strchr(line, '='); */

    /* /\* this line is a custom SQL query *\/ */
    /* if (! assign) */
    /* 	return NULL; */


}

static int get_param_type(const char *str)
{
    int i;

    for (i = 0; i < N_CONFIG_PARAMS; i++) {
	if (strcasecmp(str, glb_config_param[i].param_str) == 0)
	    return glb_config_param[i].param_const;
    }

    /* if everything else failed, let's assume it's a custom health
     * check */
    return HEALTH_CHECK;
}

static void load_int(int *i, char *value)
{
    sanitize_str(value);
    *i = strtol(value, NULL, 10);
}

static void load_str(char **str, char *value)
{
    if (value) {
	sanitize_str(value);
	*str = malloc(strlen(value)+1);
	strcpy(*str, value);
    }
    else {
	/* if the value for this parameter is NULL, just initialize it
	 * to an empty string */
	*str = malloc(1);
	strcpy(*str, "");
    }
}

static void load_parameter(config_t config, const char *line)
{
    char *param, *value, *delim = "=", buf[MAX_STR_CFG];

    /* strtok changes the original string, which we may not be
     * expecting at other points of the program */
    strcpy(buf, line);
    param = strtok(buf, delim);
    /* make sure there are no spaces */
    sanitize_str(param);
    switch(get_param_type(param))
    {
    case HTTP_PORT:
	value = strtok(NULL, delim);
	load_int(&CFG_HTTP_PORT(config), value);
    	break;
    case PG_PORT:
	value = strtok(NULL, delim);
	load_int(&CFG_PG_PORT(config), value);
    	break;
    case PG_CONNECTION_TIMEOUT:
	value = strtok(NULL, delim);
	load_int(&CFG_PG_TIMEOUT(config), value);
    	break;
    case PG_MAX_REPLICATION_LAG:
	value = strtok(NULL, delim);
	load_int(&CFG_REPLICATION_LAG(config), value);
    	break;
    case SYSLOG_FACILITY:
    	value = strtok(NULL, delim);
	load_str(&CFG_SYSLOG_FACILITY(config), value);
    	break;
    case PG_HOST:
    	value = strtok(NULL, delim);
	load_str(&CFG_PG_HOST(config), value);
    	break;
    case PG_USER:
    	value = strtok(NULL, delim);
	load_str(&CFG_PG_USER(config), value);
    	break;
    case PG_PASSWORD:
    	value = strtok(NULL, delim);
	load_str(&CFG_PG_PASSWORD(config), value);
    	break;
    case PG_DATABASE:
    	value = strtok(NULL, delim);
	load_str(&CFG_PG_DATABASE(config), value);
    	break;
    case HEALTH_CHECK:
    default:
	break;
    }

}

extern config_t config_parse(const char *file_path)
{
    FILE *fp;
    char line[MAX_STR_CFG];
    config_t config = malloc(sizeof(struct config));

    if (! config) {
	logger_write(LOG_CRIT, "%s\n", strerror(errno));
	return NULL;
    }

    if (! (fp=fopen(file_path, "r"))) {
	logger_write(LOG_CRIT, "%s: %s\n", file_path, strerror(errno));
	free(config);
	return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
    	sanitize_str(line);
    	if (strcmp(line, "") != 0) {
    	    load_parameter(config, line);
    	}
    }

    return config;
}

extern void config_cleanup(config_t config)
{
    free(CFG_SYSLOG_FACILITY(config));
    free(CFG_PG_HOST(config));
    free(CFG_PG_USER(config));
    free(CFG_PG_PASSWORD(config));
    free(CFG_PG_DATABASE(config));
    free(config);
}

extern void config_show(config_t config)
{
    printf("http_port: %d\n", CFG_HTTP_PORT(config));
    printf("syslog_facility: %s\n", CFG_SYSLOG_FACILITY(config));
    printf("pg_host: %s\n", CFG_PG_HOST(config));
    printf("pg_port: %d\n", CFG_PG_PORT(config));
    printf("pg_user: %s\n", CFG_PG_USER(config));
    printf("pg_password: %s\n", CFG_PG_PASSWORD(config));
    printf("pg_database: %s\n", CFG_PG_DATABASE(config));
    printf("pg_connection_timeout: %d\n", CFG_PG_TIMEOUT(config));
    printf("pg_max_replication_lag: %d\n", CFG_REPLICATION_LAG(config));
}
