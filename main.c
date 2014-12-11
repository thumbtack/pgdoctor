#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <libpq-fe.h>
#include <microhttpd.h>
#include "config_parser.h"
#include "logger.h"
#include "run_checks.h"
#include "strconst.h"


/* used to signal stop; is updated by the SIGTERM handler */
int glbl_stop = 0;


void create_html_page(char *page, char *body)
{
    snprintf(page, MAX_STR_HTML, STR_HTML_FMT, body);
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
    if ((strcmp(method, "GET") != 0) || (strcmp(url, "/") != 0)) {
	create_html_page(page, STR_BAD_REQUEST);
	status_code = 400;
	logger_write(LOG_ERR, "Bad request: method=%s, URL=%s\n", method, url);
    }
    else {
	check_result = run_health_checks(configuration, check_text, MAX_STR);
	create_html_page(page, check_text);
	status_code = check_result == 1 ? 200 : 500;
	logger_write(LOG_NOTICE, "Health check result: %u %s\n", status_code, check_text);
    }

    /* prepare the HTTP response and send it to the client */
    response = MHD_create_response_from_buffer(strlen(page), (void*)page, MHD_RESPMEM_MUST_COPY);
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    return ret;
}

void graceful_shutdown(int sig)
{
    /* just change the global variable; there are no race conditions
     * so this will safely break the loop in main() */
    glbl_stop = 1;
}

int main(int argc, char *argv[])
{
    struct MHD_Daemon *http_daemon;
    struct timeval tv;
    struct timeval *tvp;
    fd_set rs;
    fd_set ws;
    fd_set es;
    int max;
    unsigned MHD_LONG_LONG mhd_timeout;
    config_t config;

    /* parse configuration file */
    config = config_parse(CONFIG_FILE);
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

    /* never terminates (other than by signals, such as CTRL-C). */
    while (! glbl_stop) {
	max = 0;
	FD_ZERO(&rs);
	FD_ZERO(&ws);
	FD_ZERO(&es);
	if (MHD_get_fdset(http_daemon, &rs, &ws, &es, &max) != MHD_YES) {
	    /* fatal internal error */
	    logger_write(LOG_CRIT, "Failed to obtain select() sets\n");
	    break;
	}
	if (MHD_get_timeout(http_daemon, &mhd_timeout) == MHD_YES) {
	    tv.tv_sec = mhd_timeout / 1000;
	    tv.tv_usec = (mhd_timeout - (tv.tv_sec * 1000)) * 1000;
	    tvp = &tv;
	}
	else
	    tvp = NULL;
	select(max + 1, &rs, &ws, &es, tvp);
	MHD_run(http_daemon);
    }

    /* cleanup */
    MHD_stop_daemon(http_daemon);
    logger_close();
    config_cleanup(config);

    return 0;
}
