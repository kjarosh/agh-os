#ifndef SERVER_ACCEPT_H_
#define SERVER_ACCEPT_H_

#include "connection.h"

void setup_accept_threads(void);
void cancel_accept_threads(void);

void *thread_accept(void *args);

#endif
