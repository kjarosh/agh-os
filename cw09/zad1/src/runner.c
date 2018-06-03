#include "runner.h"

#include "logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

void cancel_threads(pthread_t *threads, int count) {
	for (int i = 0; i < count; ++i) {
		pthread_cancel(threads[i]);
	}
}

void waitfor_threads(pthread_t *threads, int count) {
	void *ret;
	for (int i = 0; i < count; ++i) {
		pthread_join(threads[i], &ret);
	}
}

#define max(a, b) ((a) > (b) ? (a) : (b))

void run(struct cl *cl, void *(*consumer)(void*), void *(*producer)(void*)) {
	pthread_t consumers[cl->consumers];
	pthread_t producers[cl->producers];
	int ids[max(cl->consumers, cl->producers)];
	
	for (int i = 0; i < cl->consumers; ++i) {
		ids[i] = i;
		if (pthread_create(&consumers[i], NULL, consumer, &ids[i]) != 0) {
			perror("Cannot create a thread");
			exit(-1);
		}
	}
	
	for (int i = 0; i < cl->producers; ++i) {
		ids[i] = i;
		if (pthread_create(&producers[i], NULL, producer, &ids[i]) != 0) {
			perror("Cannot create a thread");
			exit(-1);
		}
	}
	
	if (cl->time_limit != 0) {
		sleep(cl->time_limit);
		
		log_info("Time is up");
		
		cancel_threads(producers, cl->producers);
		cancel_threads(consumers, cl->consumers);
		log_info("Threads cancelled");
		
		waitfor_threads(producers, cl->producers);
		log_info("Producers finished");
		
		waitfor_threads(consumers, cl->consumers);
		log_info("Consumers finished");
	} else {
		waitfor_threads(producers, cl->producers);
		
		log_info("Producers finished");
		
		cancel_threads(consumers, cl->consumers);
		log_info("Consumers cancelled");
		
		waitfor_threads(consumers, cl->consumers);
		log_info("Consumers finished");
	}
}
