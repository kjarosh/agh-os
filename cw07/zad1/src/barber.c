#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "barber-shop.h"

static bool running = true;

static void sig_handler(int sig) {
	running = false;
}

static void cleanup() {
	dispose_barber();
}

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s <clients>\n", program);
}

int main(int argc, char **argv) {
	atexit(cleanup);
	signal(SIGINT, sig_handler);
	
	if (argc != 2) {
		print_help(argv[0]);
		exit(1);
	}
	
	int wr_cap = atoi(argv[1]);
	
	initialize_barber(wr_cap);
	
	while (running) {
		if (sem_wait(&bs->mx_waiting_room) != 0) {
			perror("Cannot access the waiting room");
			exit(1);
		}
		
		struct wr_seat next;
		bool customer_waiting = wr_pop(&next) == 0 ? true : false;
		
		if (customer_waiting) {
			// a customer is waiting
			
			// invite them to the chair
			sem_post(&next.waiting);
			
			sem_post(&bs->mx_waiting_room);
		} else {
			// no customers in the waiting room
			
			// go to sleep
			bs->barber_sleeping = true;
			sem_post(&bs->mx_waiting_room);
			
			if (sem_wait(&bs->sem_barber_sleeping) != 0) {
				perror("Cannot go to sleep");
				exit(1);
			}
			
			// the barber is being woken up, so the chair is already taken
		}
		
		// a customer is in the chair, so barber 'em!
		sleep(1);
		
		// tell them, we're ready
		sem_post(&bs->sem_barber_ready);
	}
}
