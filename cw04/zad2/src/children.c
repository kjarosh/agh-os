#include "children.h"

#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

static int seconds;
static pid_t ppid_main;

static void usr1_handler(int t) {
	// got permission
	
	int rt_sig = rand() % (SIGRTMAX - SIGRTMIN + 1) + SIGRTMIN;
	
	if (ppid_main != getppid()) {
		printf("ERROR: invalid ppid\n");
		exit(-1);
	}
	
	kill(ppid_main, rt_sig);
	
	exit(seconds);
}

int children_main(void) {
	ppid_main = getppid();
	srand(time(NULL) ^ (getpid() << 16));
	
	if (signal(SIGUSR1, usr1_handler) < 0) {
		perror("Cannot install handler");
		return -1;
	}
	
	signal(SIGINT, SIG_DFL);
	
	seconds = rand() % 11;
	//printf("%d: waiting %d seconds\n", (int) getpid(), seconds);
	sleep(seconds);
	
	kill(getppid(), SIGUSR1);
	
	// wait forever
	while (1)
		pause();
}
