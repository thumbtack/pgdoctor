#ifndef LOGGER_H_
#define LOGGER_H_


#include <syslog.h>
#include "config_parser.h"

extern void logger_open(config_t config);
extern void logger_write(int priority, const char *fmt, ...);
extern void logger_close();


#endif /* LOGGER_H_ */
