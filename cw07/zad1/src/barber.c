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
		
		struct wr_seat next;
		bool customer_waiting = wr_pop(&next) == 0 ? true : false;
		
		if (customer_waiting) {
			// a customer is waiting
			
			// invite them to the chair
			sem_post(&next.waiting);
			
			sem_post(mx_waiting_room);
		} else {
			// no customers in the waiting room
			
			// go to sleep
			barber_sleeping = true;
			sem_post(mx_waiting_room);
			
			if (sem_wait(sem_barber_sleeping) != 0) {
				perror("Cannot go to sleep");
				exit(1);
			}
			
			// the barber is being woken up, so the chair is already taken
		}
		
		// a customer is in the chair, so barber 'em!
		sleep(1);
		
		// tell them, we're ready
		sem_post(sem_barber_ready);
	}
}
