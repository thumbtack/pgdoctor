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


#define _DEFAULT_SOURCE /* for vsyslog */

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdarg.h>
#include "logger.h"


#define MK_LOGGER_LIST(f) \
    f(LOCAL0) f(LOCAL1) f(LOCAL2) f(LOCAL3) f(LOCAL4) \
    f(LOCAL5) f(LOCAL6) f(LOCAL7)
#define MK_LOGGER_PAIR(X) {LOG_##X, #X},


static int get_facility(config_t config)
{
    int i;
    /* map config_t strings to syslog constants */
    struct {
        int facility_const;
        char * facility_str;
    } facility[] = { MK_LOGGER_LIST(MK_LOGGER_PAIR) };

    if (! CFG_SYSLOG_FACILITY(config)) {
        return -1;
    }

    for (i = 0; i < 8; i++) {
        if (strcasecmp(CFG_SYSLOG_FACILITY(config), facility[i].facility_str) == 0) {
            return facility[i].facility_const;
        }
    }

    /* the facility is not local0..local7 */
    return -1;
}

extern void logger_open(config_t config)
{
    int facility = get_facility(config);

    if (facility == -1) {
        fprintf(stderr, "Invalid syslog facility: %s\n", CFG_SYSLOG_FACILITY(config));
        return;
    }

    openlog(NULL, LOG_CONS, facility);
}

extern void logger_write(int priority, const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vsyslog(priority, fmt, ap);
}

extern void logger_close()
{
    closelog();
}
