#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

#include "child.h"

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s <signal count> <mode>\n", program);
	printf("<mode>:\n");
	printf("\t1 -- children PIDs\n");
	printf("\t2 -- requests\n");
	printf("\t3 -- permissions\n");
}

static int usr1_received = 0;
static int sent = 0;
static int received = 0;
static pid_t pid;

void print_count() {
	printf("Received by parent: %d\n", received);
	printf("Sent:               %d\n", sent);
	exit(0);
}

static void signal_handler(int sig) {
	++received;
	
	if (sig == SIGUSR1 || sig == SIGRTMIN + 1) {
		usr1_received = 1;
	}
	
	if (sig == SIGINT) {
		if (kill(pid, SIGUSR2) >= 0) ++sent;
		
		print_count(received, sent);
	}
}

int main(int argc, char **argv) {
	if (argc != 3) {
		print_help(argv[0]);
		return 1;
	}
	
	int signal_count = atoi(argv[1]);
	int mode = atoi(argv[2]);
	
	pid = fork();
	
	if (pid == 0) {
		return child_main();
	}
	
	if (signal(SIGUSR1, signal_handler) < 0 || //
			signal(SIGRTMIN + 1, signal_handler) < 0 || //
			signal(SIGINT, signal_handler) < 0) {
		perror("Cannot install handler");
		return -1;
	}
	
	if (mode == 1) {
		for (int i = 0; i < signal_count; ++i) {
			printf("Sending SIGUSR1\n");
			if (kill(pid, SIGUSR1) >= 0) ++sent;
		}
	} else if (mode == 2) {
		sigset_t mask, oldmask, suspendmask;
		sigemptyset(&mask);
		sigfillset(&suspendmask);
		sigaddset(&mask, SIGINT);
		sigaddset(&mask, SIGUSR1);
		sigaddset(&mask, SIGRTMIN + 1);
		sigdelset(&suspendmask, SIGINT);
		sigdelset(&suspendmask, SIGUSR1);
		sigdelset(&suspendmask, SIGRTMIN + 1);
		
		for (int i = 0; i < signal_count; ++i) {
			printf("Sending SIGUSR1\n");
			
			sigprocmask(SIG_BLOCK, &mask, &oldmask);
			if (kill(pid, SIGUSR1) >= 0) {
				++sent;
				
				// wait for USR1
				while (!usr1_received)
					sigsuspend(&suspendmask);
				usr1_received = 0;
			}
			sigprocmask(SIG_SETMASK, &oldmask, NULL);
		}
	} else {
		for (int i = 0; i < signal_count; ++i) {
			printf("Sending real-time\n");
			if (kill(pid, SIGRTMIN + 1) >= 0) ++sent;
		}
	}
	
	if (kill(pid, SIGUSR2) >= 0) ++sent;
	
	print_count();
}
