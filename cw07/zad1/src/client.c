#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>

#include "barber-shop.h"

static bool running = true;

static void sig_handler(int sig) {
	running = false;
}

int main(int argc, char **argv) {
	signal(SIGINT, sig_handler);
	
	while (running) {
		if (!sem_wait(mx_waiting_room)) {
			perror("Cannot access the waiting room");
			exit(1);
		}
		
		if (barber_sleeping) {
			// no one is waiting, chair is free
			// sit in the chair then
			
			barber_chair = new_seat();
		} else {
			struct wr_seat *my_seat = wr_push();
			
			if (my_seat == NULL) {
				// no free seats
				sem_post(mx_waiting_room);
				fprintf(stderr, "No free seats\n");
				exit(-1);
			}
			
			*my_seat = new_seat();
			sem_post(mx_waiting_room);
			
			// wait in the waiting room
			sem_wait(&my_seat->waiting);
			
			// sit in the chair
			barber_chair = new_seat();
		}
		
		// wait for the haircut
		sem_wait(sem_barber_ready);
	}
}
