#include "logger.h"

#include <stdio.h>

struct cl *logger_cl;

void log_info(const char *data) {
	if(!logger_cl->verbose) return;
	
	fprintf(stderr, "%s\n", data);
}

void log_consumer(int id, const char *data) {
	if(!logger_cl->verbose) return;
	
	fprintf(stderr, "[C %d] %s\n", id, data);
}

void log_producer(int id, const char *data) {
	if(!logger_cl->verbose) return;
	
	fprintf(stderr, "[P %d] %s\n", id, data);
}
