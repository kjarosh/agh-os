#include "child.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

static int received_count = 0;

static void signal_handler(int sig) {
	++received_count;
	
	if (sig == SIGUSR1 || sig == SIGRTMIN + 1) {
		printf("Received signal 1, replying\n");
		kill(getppid(), sig);
	}
	
	if (sig == SIGUSR2 || sig == SIGRTMIN + 2) {
		printf("Received signal 2, exiting\n");
		printf("Received by child:  %d\n", received_count);
		exit(0);
	}
}

static void setup_signals(void) {
	// ignore all signals except usr1 & usr2
	sigset_t set;
	sigfillset(&set);
	sigdelset(&set, SIGUSR1);
	sigdelset(&set, SIGUSR2);
	sigdelset(&set, SIGRTMIN + 1);
	sigdelset(&set, SIGRTMIN + 2);
	sigprocmask(SIG_SETMASK, &set, NULL);
	
	// handle usr1
	if (signal(SIGUSR1, signal_handler) < 0) {
		perror("Cannot install handler");
		exit(-1);
	}
	if (signal(SIGRTMIN + 1, signal_handler) < 0) {
		perror("Cannot install handler");
		exit(-1);
	}
	
	// handle usr2
	if (signal(SIGUSR2, signal_handler) < 0) {
		perror("Cannot install handler");
		exit(-1);
	}
	// handle usr2
	if (signal(SIGRTMIN + 2, signal_handler) < 0) {
		perror("Cannot install handler");
		exit(-1);
	}
}

int child_main(void) {
	setup_signals();
	
	while (1)
		pause();
	
	return 0;
}
