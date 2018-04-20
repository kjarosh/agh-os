#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

#include "children.h"

typedef enum chld_state {
	RUNNING = 0, REQUESTED, TERMINATED
} chld_state;

static int request_count = 0;
static int request_threshold = 0;

static int info_flags = 0;

static int children_count = 0;
static pid_t *children;
static chld_state *children_states;

enum {
	INFO_CHLDPID = 0x10,
	INFO_REQUESTS = 0x08,
	INFO_PERMS = 0x04,
	INFO_SIGRT = 0x02,
	INFO_TERM = 0x01
};

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s <children count> <request threshold> <info>\n", program);
	printf("<info>:\n");
	printf("\t1xxxx: children PIDs\n");
	printf("\tx1xxx: requests\n");
	printf("\txx1xx: permissions\n");
	printf("\txxx1x: SIGRT\n");
	printf("\txxxx1: children termination\n");
}

int all_children_terminated(void) {
	for (int i = 0; i < children_count; ++i) {
		if (children_states[i] != TERMINATED) {
			return 0;
		}
	}
	
	return 1;
}

static void int_handler(int t) {
	printf("Got interrupt, terminating children...\n");
	
	for (int i = 0; i < children_count; ++i) {
		if (children_states[i] != TERMINATED) {
			children_states[i] = TERMINATED;
			kill(children[i], SIGKILL);
		}
	}
	
	exit(0);
}

static void rt_handler(int t, siginfo_t *siginf, void *ign) {
	if (info_flags & INFO_SIGRT) {
		printf("Got real-time signal %d from PID %d\n", t - SIGRTMIN, (int) (siginf->si_pid));
	}
}

// request
static void usr1_handler(int t, siginfo_t *siginf, void *ign) {
	pid_t sender = siginf->si_pid;
	
	if (info_flags & INFO_REQUESTS) {
		printf("Got request from PID %d\n", (int) sender);
	}
	
	++request_count;
	
	for (int i = 0; i < children_count; ++i) {
		if (children[i] == sender) {
			children_states[i] = REQUESTED;
		}
	}
	
	if (request_count < request_threshold) {
		return;
	}
	
	for (int i = 0; i < children_count; ++i) {
		if (children_states[i] == REQUESTED) {
			children_states[i] = RUNNING;
			
			kill(children[i], SIGUSR1);
			if (info_flags & INFO_PERMS) {
				printf("Gave permissions to %d\n", (int) children[i]);
			}
		}
	}
}

static void run() {
	while (1) {
		int status;
		pid_t child_pid = wait(&status);
		if (child_pid < 0) {
			if (errno == EINTR)
				continue;
			else
				return;
		}
		
		if (info_flags & INFO_TERM) {
			printf("Child terminated with PID %d and status %d\n", (int) child_pid,
					WEXITSTATUS(status));
		}
		
		for (int i = 0; i < children_count; ++i) {
			if (children[i] == child_pid) {
				children_states[i] = TERMINATED;
			}
		}
		
		if (all_children_terminated()) {
			exit(0);
		}
	}
}

void setup_signals(void) {
	struct sigaction act;
	act.sa_sigaction = usr1_handler;
	act.sa_flags = SA_SIGINFO;
	sigemptyset(&act.sa_mask);
	if (sigaction(SIGUSR1, &act, NULL) < 0) {
		perror("Cannot install handler");
		exit(-1);
	}
	
	if (signal(SIGINT, int_handler) < 0) {
		perror("Cannot install handler");
		exit(-1);
	}
	
	act.sa_sigaction = rt_handler;
	act.sa_flags = SA_SIGINFO;
	for (int i = SIGRTMIN; i <= SIGRTMAX; ++i) {
		if (sigaction(i, &act, NULL) < 0) {
			perror("Cannot install handler");
			exit(-1);
		}
	}
}

int main(int argc, char **argv) {
	srand(time(NULL));
	
	if (argc != 4) {
		print_help(argv[0]);
		return 1;
	}
	
	setup_signals();
	
	children_count = atoi(argv[1]);
	request_threshold = atoi(argv[2]);
	info_flags = strtol(argv[3], NULL, 2);
	//printf("%#04x\n", info_flags);
	
	if (request_threshold > children_count) {
		fprintf(stderr, "Error: threshold too high\n");
		exit(-1);
	}
	
	pid_t children0[children_count];
	chld_state children_states0[children_count];
	children = &children0[0];
	children_states = &children_states0[0];
	
	for (int i = 0; i < children_count; ++i) {
		children_states[i] = RUNNING;
	}
	
	for (int i = 0; i < children_count; ++i) {
		pid_t pf = fork();
		if (pf < 0) {
			perror("Failed to fork");
			--i;
			continue;
		}
		
		if (pf == 0) {
			return children_main();
		}
		
		children[i] = pf;
		
		if (info_flags & INFO_CHLDPID) {
			printf("Child created with PID %d\n", (int) children[i]);
		}
	}
	
	run();
}
