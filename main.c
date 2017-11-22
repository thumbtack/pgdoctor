/*
 Copyright 2014-2017 Thumbtack, Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/


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
    config_destroy(config);

    return 0;
}
