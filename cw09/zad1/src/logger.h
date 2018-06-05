#ifndef LOGGER_H_
#define LOGGER_H_

#include "cl.h"

extern struct cl *logger_cl;

void log_info(const char *data);
void log_producer(int id, const char *data);
void log_consumer(int id, const char *data);

#endif
