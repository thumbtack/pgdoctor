#ifndef LOGGER_H
#define LOGGER_H


#include <syslog.h>
#include "config_parser.h"

extern void logger_open(config_t config);
extern void logger_write(int priority, const char *fmt, ...);
extern void logger_close();


#endif
