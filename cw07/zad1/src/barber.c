#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "barber-shop.h"

static int clients_count = 0;
static int haircuts_no = 0;

static bool running = true;

static void spawn_clients() {
	for (int i = 0; i < clients_count; ++i) {
		pid_t forked = fork();
		
		if (forked != 0) {
			// barber
		} else {
			char num[128];
			sprintf(num, "%d", haircuts_no);
			
			// client
			execl("./client", "client", num, (char*) NULL);
			perror("Cannot exec");
			_exit(-1);
		}
	}
}

static pid_t listener_pid = -1;

static void sig_handler(int sig) {
	if (listener_pid > 0) {
		fprintf(stderr, "Barber received signal %d\n", sig);
		kill(listener_pid, SIGTERM);
	}
	
	exit(0);
}

static void cleanup() {
	if (listener_pid > 0) {
		dispose_barber();
	}
}

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s <clients> <haircuts>\n", program);
}

int main(int argc, char **argv) {
	atexit(cleanup);
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);
	
#ifdef SYS_V
	printf("Hello from barber %d at System V!\n", getpid());
#else
	printf("Hello from barber %d at POSIX!\n", getpid());
#endif
	
	if (argc != 3) {
		print_help(argv[0]);
		exit(1);
	}
	
	int wr_cap = atoi(argv[1]);
	int haircuts = atoi(argv[2]);
	
	initialize_barber(wr_cap);
	
	clients_count = wr_cap;
	haircuts_no = haircuts;
	
	listener_pid = fork();
	if (listener_pid == 0) {
		atexit(cleanup);
		spawn_clients();
		
		while (true) {
			int gc = getchar();
			if (gc == 's') {
				fprintf(stderr, "Spawning\n");
				spawn_clients();
			} else if (gc == EOF) {
				fprintf(stderr, "In order to interrupt press ^C\n");
			}
		}
	}
	
	while (running) {
		if (semaphore_wait(conv_tp(bs->mx_waiting_room)) != 0) {
			perror("[B] Cannot access the waiting room");
			exit(1);
		}
		
		struct wr_seat *next;
		bool customer_waiting = wr_pop(&next) == 0 ? true : false;
		
		if (customer_waiting) {
			// a customer is waiting
			
			// invite them to the chair
			log_barber2("Inviting client", next->pid);
			semaphore_post(conv_tp(next->waiting));
			
			semaphore_post(conv_tp(bs->mx_waiting_room));
		} else {
			// no customers in the waiting room
			
			// go to sleep
			bs->barber_sleeping = true;
			semaphore_post(conv_tp(bs->mx_waiting_room));
			
			log_barber("Going to sleep");
			do {
				if (semaphore_wait(conv_tp(bs->sem_barber_sleeping)) != 0) {
					// if it's the interrupt
					if (errno == EINTR) continue;
					
					perror("[B] Cannot sleep");
					exit(1);
				} else {
					break;
				}
			} while (errno == EINTR);
			
			// the barber is being woken up, so the chair is already taken
			log_barber("Waking up");
		}
		
		semaphore_wait2(conv_tp(bs->sem_customer_ready));
		
		log_barber2("Starting haircut for", bs->barber_chair.pid);
		
		// a customer is in the chair, so barber 'em!
		
		log_barber2("Haircut ready for", bs->barber_chair.pid);
		
		// tell them, we're ready
		semaphore_post(conv_tp(bs->sem_barber_ready));
		
		// wait for them to get out
		semaphore_wait2(conv_tp(bs->sem_customer_ready));
	}
}
