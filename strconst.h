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


#ifndef STRCONST_H_
#define STRCONST_H_


#define MAX_STR 256
#define MAX_STR_CFG 1024
#define MAX_STR_HTML 4096

#ifdef DEBUG
#define CONFIG_FILE "pgdoctor.cfg"
#else
#define CONFIG_FILE "/etc/pgdoctor.cfg"
#endif

#define CFG_COMMENT_CHR '#'
#define CFG_DELIMITER_CHR "="

#define STR_PG_CONN_INFO_FMT "host=%s port=%d dbname=%s user=%s " \
                             "password=%s connect_timeout=%d"
#define STR_REPLICATION_LAG_QUERY "SELECT extract('epoch' from " \
                                  "(now()-pg_last_xact_replay_timestamp())) " \
                                  "AS replication_lag;"
#define STR_HTML_RESPONSE_FMT "<html><body>%s</body></html>"
#define STR_BAD_REQUEST_FMT "Bad request: method=%s, URL=%s\n"
#define STR_HTTP_RESPONSE_FMT "Health check result: %u %s\n"
#define STR_QUERY_FAILED_FMT "Query returned no result: '%s'"
#define STR_CFG_PARSE_ERROR_FMT "Failed to parse line '%s'\n"
#define STR_CFG_PARSE_SUCCESS_FMT "Successfully parsed line '%s'\n"
#define STR_DB_CONNECTION_ERROR "Failed to connect to the database\n"
#define STR_HEALTH_CHECK_ERROR_DETAIL_FMT "Check failed: '%s %s %s'. Return value was %s\n"
#define STR_HEALTH_CHECK_ERROR_FMT "Check failed: '%s %s %s'\n"
#define STR_HEALTH_CHECK_SUCCESS_FMT "Check succeeded: '%s %s %s'\n"
#define STR_ALL_CHECKS_SUCCESSFUL "OK"


#endif				/* STRCONST_H_ */
