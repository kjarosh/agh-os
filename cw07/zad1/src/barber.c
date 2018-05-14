#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "barber-shop.h"

static int clients_count = 0;
static pid_t *clients = NULL;
static bool running = true;

static void sig_handler(int sig) {
	fprintf(stderr, "Barber received signal\n");
	running = false;
}

static void cleanup() {
	dispose_barber();
	
	for (int i = 0; i < clients_count; ++i) {
		kill(clients[i], SIGTERM);
	}
	
	free(clients);
}

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s <clients> <haircuts>\n", program);
}

static void spawn_clients(int count, int haircuts) {
	clients = malloc(count * sizeof(pid_t));
	clients_count = count;
	
	for (int i = 0; i < count; ++i) {
		pid_t forked = fork();
		
		if (forked != 0) {
			// barber
			clients[i] = forked;
		} else {
			char num[128];
			sprintf(num, "%d", haircuts);
			
			// client
			execl("./client", "client", num, (char*) NULL);
			perror("Cannot exec");
			_exit(-1);
		}
	}
}

int main(int argc, char **argv) {
	atexit(cleanup);
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	
	if (argc != 3) {
		print_help(argv[0]);
		exit(1);
	}
	
	int wr_cap = atoi(argv[1]);
	int haircuts = atoi(argv[2]);
	
	initialize_barber(wr_cap);
	
	spawn_clients(wr_cap, haircuts);
	
	while (running) {
		if (sem_wait(&bs->mx_waiting_room) != 0) {
			perror("Cannot access the waiting room");
			exit(1);
		}
		
		struct wr_seat *next;
		bool customer_waiting = wr_pop(&next) == 0 ? true : false;
		
		if (customer_waiting) {
			// a customer is waiting
			
			// invite them to the chair
			log_barber2("Inviting client", next->pid);
			sem_post(&next->waiting);
			
			sem_post(&bs->mx_waiting_room);
		} else {
			// no customers in the waiting room
			
			// go to sleep
			bs->barber_sleeping = true;
			sem_post(&bs->mx_waiting_room);
			
			log_barber("Going to sleep");
			if (sem_wait(&bs->sem_barber_sleeping) != 0) {
				if (errno == EINTR) exit(0);
				
				perror("Cannot go to sleep");
				exit(1);
			}
			
			// the barber is being woken up, so the chair is already taken
			log_barber("Waking up");
			bs_sleep(1);
		}
		
		sem_wait(&bs->sem_customer_ready);
		
		log_barber2("Starting haircut for", bs->barber_chair.pid);
		
		// a customer is in the chair, so barber 'em!
		
		log_barber2("Haircut ready for", bs->barber_chair.pid);
		
		// tell them, we're ready
		sem_post(&bs->sem_barber_ready);
		
		// wait for them to get out
		sem_wait(&bs->sem_customer_ready);
	}
}
