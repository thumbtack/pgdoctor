#ifndef STRCONST_H_
#define STRCONST_H_


#define MAX_STR 128
#define MAX_STR_CFG 1024
#define MAX_STR_HTML 4096

#ifdef DEBUG
#define CONFIG_FILE "pgdoctor.cfg"
#else
#define CONFIG_FILE "/etc/pgdoctor.cfg"
#endif

#define COMMENT_CHR '#'

#define STR_PG_CONN_INFO_FMT "host=%s port=%d dbname=%s user=%s " \
                             "password=%s connect_timeout=%d"
#define STR_REPLICATION_LAG_QUERY "SELECT extract(seconds from " \
                                  "(now()-pg_last_xact_replay_timestamp())) " \
                                  "AS replication_lag;"
#define STR_HTML_RESPONSE_FMT "<html><body>%s</body></html>"
#define STR_BAD_REQUEST_FMT "Bad request: method=%s, URL=%s\n"
#define STR_HTTP_RESPONSE_FMT "Health check result: %u %s\n"
#define STR_QUERY_FAILED_FMT "Query returned no result: '%s'"
#define STR_CFG_PARSE_ERROR_FMT "Failed to parse line '%s'\n"
#define STR_CFG_PARSE_SUCCESS_FMT "Successfully parsed line '%s'\n"
#define STR_DB_CONNECTION_ERROR "Failed to connect to the database\n"
#define STR_HEALTH_CHECK_ERROR_FMT "Check failed: '%s %s %s'\n"
#define STR_HEALTH_CHECK_SUCCESS_FMT "Check succeeded: '%s %s %s'\n"
#define STR_ALL_CHECKS_SUCCESSFUL "OK"


#endif /* STRCONST_H_ */
