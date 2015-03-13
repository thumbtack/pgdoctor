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


#define _XOPEN_SOURCE 500 /* for usleep */


#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <microhttpd.h>
#include <libpq-fe.h>
#include "config_parser.h"
#include "logger.h"
#include "run_checks.h"
#include "strconst.h"


/* used to signal stop; is updated by the SIGTERM handler */
int global_stop = 0;


void create_html_page(char *page, size_t size, const char *body)
{
    snprintf(page, size, STR_HTML_RESPONSE_FMT, body);
}

int answer_to_connection(void *cls,
			 struct MHD_Connection *connection,
			 const char *url,
			 const char *method,
			 const char *version,
			 const char *upload_data,
			 size_t *upload_data_size,
			 void **con_cls)
{
    char check_text[MAX_STR], page[MAX_STR_HTML];
    struct MHD_Response *response;
    int ret, check_result;
    unsigned int status_code;
    config_t configuration = (config_t)cls;

    /* unless the request is a simple GET to /, just ignore it */
    if ((strcmp(method, "GET") == 0) && (strcmp(url, "/") == 0)) {
	check_result = run_health_checks(configuration,
					 check_text,
					 sizeof(check_text));
	status_code = check_result == 1 ? 200 : 500;
	create_html_page(page, sizeof(page), check_text);
	logger_write(LOG_NOTICE, STR_HTTP_RESPONSE_FMT,
		     status_code, check_text);
    } else {
	snprintf(check_text, sizeof(check_text),
		 STR_BAD_REQUEST_FMT, method, url);
	status_code = 400;
	create_html_page(page, sizeof(page), check_text);
	logger_write(LOG_ERR, check_text);
    }

    /* prepare the HTTP response and send it to the client */
    response = MHD_create_response_from_buffer(strlen(page), (void*)page,
					       MHD_RESPMEM_MUST_COPY);
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    return ret;
}

void graceful_shutdown(int sig)
{
    /* just change the global variable; there are no race conditions
     * so this will safely break the loop in main() */
    global_stop = 1;
}

int main(int argc, char *argv[])
{
    struct MHD_Daemon *http_daemon;
    config_t config;

    /* parse configuration file; allow a single command line argument
     * with the path to the configuration file */
    if (argc == 2) {
	config = config_parse(argv[1]);
    } else {
	config = config_parse(CONFIG_FILE);
    }

    if (! config) {
	fprintf(stderr, "Failed to parse configuration file: %s\n", CONFIG_FILE);
	return 1;
    }

    /* setup the logger */
    logger_open(config);

    /* setup a signal handler to break the daemon loop and cleanup
     * nicely on SIGTERM */
    signal(SIGTERM, graceful_shutdown);

    /* run daemon */
    http_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY,
				   CFG_HTTP_PORT(config), NULL, NULL,
				   &answer_to_connection, config,
				   MHD_OPTION_END);
    if (http_daemon == NULL) {
	logger_write(LOG_CRIT, "Failed to start HTTP daemon\n");
	return 1;
    }

    /* the server daemon runs in the background in its own thread, so
     * the execution flow in our main function would contine right
     * after the call and the program would exit */
    while (! global_stop) {
      usleep(500000);
    }

    /* cleanup */
    MHD_stop_daemon(http_daemon);
    logger_close();
    config_cleanup(config);

    return 0;
}
