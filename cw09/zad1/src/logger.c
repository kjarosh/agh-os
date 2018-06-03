#include  "logger.h"

#include <stdio.h>

void log_info(const char *data) {
	fprintf(stderr, "%s\n", data);
}

void log_consumer(int id, const char *data) {
	fprintf(stderr, "[C %d] %s\n", id, data);
}

void log_producer(int id, const char *data) {
	fprintf(stderr, "[P %d] %s\n", id, data);
}
