#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "barber-shop.h"

static bool running = true;

static void sig_handler(int sig) {
	running = false;
}

static void cleanup() {
	dispose_client();
}

int main(int argc, char **argv) {
	atexit(cleanup);
	signal(SIGINT, sig_handler);
	
	initialize_client();
	
	while (running) {
		if (sem_wait(&bs->mx_waiting_room) != 0) {
			perror("Cannot access the waiting room");
			exit(1);
		}
		
		if (bs->barber_sleeping) {
			// no one is waiting, chair is free
			// sit in the chair then
			
			bs->barber_chair = new_seat();
		} else {
			struct wr_seat *my_seat = wr_push();
			
			if (my_seat == NULL) {
				// no free seats
				sem_post(&bs->mx_waiting_room);
				fprintf(stderr, "No free seats\n");
				exit(-1);
			}
			
			*my_seat = new_seat();
			sem_post(&bs->mx_waiting_room);
			
			// wait in the waiting room
			sem_wait(&my_seat->waiting);
			
			// sit in the chair
			bs->barber_chair = new_seat();
		}
		
		// wait for the haircut
		sem_wait(&bs->sem_barber_ready);
	}
}
