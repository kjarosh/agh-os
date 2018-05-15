#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "barber-shop.h"

static void sig_handler(int sig) {
	exit(0);
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
	
#ifdef BS_SLEEP
	usleep(rand() % 1000000);
#endif
	
	atexit(cleanup);
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	
	if (argc != 2) {
		print_help(argv[0]);
		exit(1);
	}
	
	int haircuts = atoi(argv[1]);
	
	initialize_client();
	
	while (haircuts > 0) {
		--haircuts;
		
		if (semaphore_wait(conv_tp(bs->mx_waiting_room)) != 0) {
			perror("[C] Cannot access the waiting room");
			exit(1);
		}
		
		if (bs->barber_sleeping) {
			// no one is waiting, chair is free
			// sit in the chair then
			
			log_client("Waking up the barber");
			bs->barber_sleeping = false;
			semaphore_post(conv_tp(bs->sem_barber_sleeping));
			semaphore_post(conv_tp(bs->mx_waiting_room));
			
			log_client("Sitting in the chair");
			bs->barber_chair = new_seat();
			
			semaphore_post(conv_tp(bs->sem_customer_ready));
		} else {
			struct wr_seat *my_seat = wr_push();
			
			if (my_seat == NULL) {
				// no free seats
				semaphore_post(conv_tp(bs->mx_waiting_room));
				log_client("No free seats");
				exit(-1);
			}
			
			log_client("Going to the waiting room");
			*my_seat = new_seat();
			semaphore_post(conv_tp(bs->mx_waiting_room));
			
			// wait in the waiting room
			semaphore_wait2(conv_tp(my_seat->waiting));
			
			// sit in the chair
			log_client("Sitting in the chair");
			bs->barber_chair = new_seat();
			
			delete_seat(my_seat);
			
			semaphore_post(conv_tp(bs->sem_customer_ready));
		}
		
		// wait for the haircut
		if (semaphore_wait(conv_tp(bs->sem_barber_ready)) != 0) {
			perror("[C] Cannot wait for barber");
		}
		
		log_client("Haircut done. Thanks!");
		
		delete_seat(&bs->barber_chair);
		bs->barber_chair = empty_seat();
		
		semaphore_post(conv_tp(bs->sem_customer_ready));
		
		bs_sleep(rand() % 6 + 1);
	}
}
