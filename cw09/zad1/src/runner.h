#ifndef RUNNER_H_
#define RUNNER_H_

#include "cl.h"

void run(struct cl *cl, void *(*consumer)(void*), void *(*producer)(void*));

#endif
