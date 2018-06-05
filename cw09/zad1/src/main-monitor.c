#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "cl.h"
#include "buffer.h"
#include "runner.h"
#include "logger.h"

struct cl cl;
pthread_mutex_t buffer_mx;
pthread_cond_t producer_line;
pthread_cond_t consumer_line;

void unlock_mx(void *args) {
	pthread_mutex_unlock(&buffer_mx);
}

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
		
		pthread_testcancel();
		
		if (pthread_mutex_lock(&buffer_mx) != 0) {
			perror("Cannot lock a mutex");
			exit(-1);
		}
		
		while (buffer_push(line) != 0) {
			log_producer(id, "Buffer is full, waiting...");
			
			pthread_cleanup_push(unlock_mx, NULL);
			if (pthread_cond_wait(&producer_line, &buffer_mx) != 0) {
				perror("Cannot wait on condition");
				exit(-1);
			}
			pthread_cleanup_pop(0);
		}
		
		log_producer(id, "Pushed");
		
		pthread_cond_signal(&consumer_line);
		
		if (pthread_mutex_unlock(&buffer_mx) != 0) {
			perror("Cannot unlock a mutex");
			exit(-1);
		}
		
		line = NULL;
		n = 0;
		
		if(cl.sleep > 0){
			sleep(cl.sleep);
		}
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
		if (pthread_mutex_lock(&buffer_mx) != 0) {
			perror("Cannot lock a mutex");
			exit(-1);
		}
		
		char *line;
		while (buffer_pop(&line) != 0) {
			log_consumer(id, "Buffer empty, waiting...");
			
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
			pthread_cleanup_push(unlock_mx, NULL);
			if (pthread_cond_wait(&consumer_line, &buffer_mx) != 0) {
				perror("Cannot wait on condition");
				exit(-1);
			}
			pthread_cleanup_pop(0);
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		}
		
		pthread_cond_signal(&producer_line);
		log_consumer(id, "Popped");
		
		if (pthread_mutex_unlock(&buffer_mx) != 0) {
			perror("Cannot unlock a mutex");
			exit(-1);
		}
		
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
	pthread_cond_destroy(&producer_line);
	pthread_cond_destroy(&consumer_line);
	pthread_mutex_destroy(&buffer_mx);
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
	
	if (pthread_mutex_init(&buffer_mx, NULL) != 0) {
		perror("Cannot initialize a mutex");
		exit(-1);
	}
	
	if (pthread_cond_init(&consumer_line, NULL) != 0
			|| pthread_cond_init(&producer_line, NULL) != 0) {
		perror("Cannot initialize a cond");
		exit(-1);
	}
	
	run(&cl, consumer, producer);
	
	log_info("Done");
	return 0;
}
