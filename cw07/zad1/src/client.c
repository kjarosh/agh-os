#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "barber-shop.h"

static bool running = true;

static void sig_handler(int sig) {
	running = false;
}

static void cleanup() {
	dispose_client();
}

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s <haircuts>\n", program);
}

int main(int argc, char **argv) {
	srand(time(NULL) ^ getpid());
	
	atexit(cleanup);
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	
	if (argc != 2) {
		print_help(argv[0]);
		exit(1);
	}
	
	int haircuts = atoi(argv[1]);
	
	initialize_client();
	
	while (running && haircuts > 0) {
		--haircuts;
		
		if (sem_wait(&bs->mx_waiting_room) != 0) {
			perror("Cannot access the waiting room");
			exit(1);
		}
		
		if (bs->barber_sleeping) {
			// no one is waiting, chair is free
			// sit in the chair then
			
			log_client("Waking up the barber");
			bs->barber_sleeping = false;
			sem_post(&bs->sem_barber_sleeping);
			sem_post(&bs->mx_waiting_room);
			
			log_client("Sitting in the chair");
			bs->barber_chair = new_seat();
			
			sem_post(&bs->sem_customer_ready);
		} else {
			struct wr_seat *my_seat = wr_push();
			
			if (my_seat == NULL) {
				// no free seats
				sem_post(&bs->mx_waiting_room);
				log_client("No free seats");
				exit(-1);
			}
			
			log_client("Going to the waiting room");
			*my_seat = new_seat();
			sem_post(&bs->mx_waiting_room);
			
			// wait in the waiting room
			sem_wait(&my_seat->waiting);
			
			// sit in the chair
			log_client("Sitting in the chair");
			bs->barber_chair = new_seat();
			
			sem_post(&bs->sem_customer_ready);
		}
		
		// wait for the haircut
		if (sem_wait(&bs->sem_barber_ready) != 0) {
			perror("Cannot wait for barber");
		}
		
		log_client("Haircut done. Thanks!");
		
		bs->barber_chair = empty_seat();
		
		sem_post(&bs->sem_customer_ready);
		
		bs_sleep(rand() % 4 + 1);
	}
}
