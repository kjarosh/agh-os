#ifndef LOGGER_H_
#define LOGGER_H_

void log_info(const char *data);
void log_producer(int id, const char *data);
void log_consumer(int id, const char *data);

#endif
