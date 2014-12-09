#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H


#include "health_check.h"

typedef struct checks_list {
    health_check_t check;
    struct checks_list *next;
} *t_checks_list;

typedef struct config {
    int http_port;
    char *syslog_facility;
    char *host;
    int port;
    char *user;
    char *password;
    char *database;
    int connection_timeout;
    int replication_lag;
    t_checks_list checks;
} *config_t;


/* a bit of an overkill since this really isn't an abstract data type,
 * but the definition has changed so many times that these macros just
 * make everything a lot easier */
#define CFG_HTTP_PORT(X) (X->http_port)
#define CFG_SYSLOG_FACILITY(X) (X->syslog_facility)
#define CFG_PG_HOST(X) (X->host)
#define CFG_PG_PORT(X) (X->port)
#define CFG_PG_USER(X) (X->user)
#define CFG_PG_PASSWORD(X) (X->password)
#define CFG_PG_DATABASE(X) (X->database)
#define CFG_PG_TIMEOUT(X) (X->connection_timeout)
#define CFG_REPLICATION_LAG(X) (X->replication_lag)
#define CFG_CUSTOM_CHECK(X) (X->custom_check)

extern void sanitize_str(char *str);
extern config_t config_parse(const char *file_path);
extern void config_cleanup(config_t config);
extern void config_show(config_t config);


#endif
