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

#define STR_PG_CONN_INFO_FMT "host=%s port=%d dbname=%s user=%s " \
                             "password=%s connect_timeout=%d"
#define STR_REPLICATION_LAG_QUERY "SELECT extract(seconds from " \
                                  "(now()-pg_last_xact_replay_timestamp())) " \
                                  "AS replication_lag;"
#define STR_HTML_FMT "<html><body>%s</body></html>"
#define STR_OK "OK"
#define STR_NO_REPLICATION_INFO "Unknown replication state"
#define STR_QUERY_FAILED "Failed to execute query '%s'"
#define STR_BAD_REQUEST "Bad request"
#define STR_CFG_PARSE_ERROR "Failed to parse line '%s'\n"
#define STR_CFG_PARSE_SUCCESS "Successfully parsed line '%s'\n"
#define STR_DB_CONNECTION_ERROR "Failed to connect to the database\n"
#define STR_RUN_CHECK_ERROR "Failed to run check '%s %s %s'\n"
#define STR_RUN_CHECK_SUCCESS "Successfully ran check '%s %s %s'\n"

#define COMMENT_CHR '#'


#endif /* STRCONST_H_ */
