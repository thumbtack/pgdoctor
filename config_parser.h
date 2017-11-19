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


#ifndef CONFIG_PARSER_H_
#define CONFIG_PARSER_H_


#include "custom_check.h"

/* map strings from the configuration file to equivalent constants */
#define MK_CONFIG_LIST(f) \
    f(HTTP_PORT) f(SYSLOG_FACILITY) f(PG_HOST) f(PG_PORT) f(PG_USER) \
    f(PG_PASSWORD) f(PG_DATABASE) f(PG_CONNECTION_TIMEOUT) \
    f(PG_MAX_REPLICATION_LAG) f(CUSTOM_CHECK)
#define N_CONFIG_PARAMS 10
#define MK_CONFIG_PAIR(X) {X, #X},
#define MK_CONFIG_ENUM(X) X,

typedef enum { MK_CONFIG_LIST(MK_CONFIG_ENUM) } config_option_t;

typedef struct checks_list {
    custom_check_t check;
    struct checks_list *next;
} *checks_list_t;

typedef struct config {
    int http_port;
    char *syslog_facility;
    char *host;
    int port;
    char *user;
    char *password;
    char *database;
    int connection_timeout;
    checks_list_t custom_checks;
} *config_t;

#define CHECKS_LIST_CHECK(X) (X->check)
#define CHECKS_LIST_NEXT(X) (X->next)

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
#define CFG_CUSTOM_CHECKS(X) (X->custom_checks)

extern void sanitize_str(char *str);
extern config_t config_parse(const char *file_path);
extern void config_cleanup(config_t config);
extern void config_show(config_t config);


#endif				/* CONFIG_PARSER_H_ */
