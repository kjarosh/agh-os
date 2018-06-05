#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "cl.h"
#include "buffer.h"
#include "runner.h"
#include "logger.h"

#define sem_wait_f(sem) \
	if (sem_wait(sem) != 0) { \
		perror("Cannot wait"); \
		exit(-1); \
	}

#define sem_post_f(sem) do { \
		if (sem_post(sem) != 0) { \
			perror("Cannot post"); \
			exit(-1); \
		} \
	} while(0)

struct cl cl;
sem_t buffer_mx;
sem_t producer_line;
sem_t consumer_line;

void *producer(void *data) {
	int id = *((int*) data);
	
	if(cl.sleep > 0){
		usleep(rand() % (cl.sleep * 1000000));
	}
	
	log_producer(id, "Initializing");
	
	FILE *input = fopen(cl.filename, "r");
	if (input == NULL) {
		perror("Cannot open file from producer");
		return NULL;
	}
	
	log_producer(id, "File opened, reading...");
	
	char *line = NULL;
	size_t n = 0;
	while (getline(&line, &n, input) != -1) {
		line = strtok(line, "\n");
		if (line == NULL) continue;
		
		sem_wait_f(&buffer_mx);
		
		while (buffer_full()) {
			sem_post_f(&buffer_mx);
			
			log_producer(id, "Buffer is full, waiting...");
			sem_wait_f(&producer_line);
			
			sem_wait_f(&buffer_mx);
		}
		
		buffer_push(line);
		sem_post_f(&consumer_line);
		log_producer(id, "Pushed");
		
		sem_post_f(&buffer_mx);
		
		line = NULL;
		n = 0;
	}
	
	log_producer(id, "No more lines to produce, exiting...");
	
	return NULL;
}

void *consumer(void *data) {
	int id = *((int*) data);
	
	if(cl.sleep > 0){
		usleep(rand() % (cl.sleep * 1000000));
	}
	
	log_consumer(id, "Initializing");
	
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	while (1) {
		sem_wait_f(&buffer_mx);
		
		while (buffer_empty()) {
			sem_post_f(&buffer_mx);
			
			log_consumer(id, "Buffer empty, waiting...");
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			sem_wait_f(&consumer_line);
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
			
			sem_wait_f(&buffer_mx);
		}
		
		char *line;
		buffer_pop(&line);
		sem_post_f(&producer_line);
		log_consumer(id, "Popped");
		
		sem_post_f(&buffer_mx);
		
		if (cl_shouldprint(&cl, line)) {
			printf("%s\n", line);
		}
		
		if(cl.sleep > 0){
			sleep(cl.sleep);
		}
		
		free(line);
	}
	
	return NULL;
}

void sighandler(int sig){
	exit(0);
}

void cleanup(){
	sem_destroy(&producer_line);
	sem_destroy(&consumer_line);
	sem_destroy(&buffer_mx);
}

int main(int argc, char **argv) {
	srand(time(NULL));
	signal(SIGINT, sighandler);
	atexit(cleanup);
	
	logger_cl = &cl;
	if (cl_initialize(&cl, argc, argv) != 0) {
		fprintf(stderr, "Failed to initialize command line\n");
		return -1;
	}
	
	buffer_init(cl.buffer_size);
	
	if (sem_init(&buffer_mx, 0, 1) != 0 || sem_init(&producer_line, 0, 0) != 0
			|| sem_init(&consumer_line, 0, 0) != 0) {
		perror("Cannot initialize a semaphore");
		exit(-1);
	}
	
	run(&cl, consumer, producer);
	
	log_info("Done");
	return 0;
}
