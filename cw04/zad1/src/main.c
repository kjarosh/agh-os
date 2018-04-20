#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>

static pid_t child_pid = -1;

void print_time(void) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	printf("time: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void tstp_handler(int t) {
	if (child_pid > 0) {
		kill(child_pid, SIGKILL);
		child_pid = -1;
		
		printf("Stopped. Waiting for:\n");
		printf("\t^Z -- continue\n");
		printf("\t^C -- interrupt\n");
	} else {
		child_pid = fork();
		
		if (child_pid == 0) {
			execl("/bin/bash", "bash", "showdate.sh", NULL);
			exit(-1);
		}
	}
}

void int_handler(int t) {
	printf("Received SIGINT.\n");
	exit(0);
}

int main(int argc, char **argv) {
	struct sigaction act;
	act.sa_handler = tstp_handler;
	if (sigaction(SIGTSTP, &act, NULL) < 0) {
		perror("Cannot install SIGTSTP handler");
		return -1;
	}
	
	if (signal(SIGINT, int_handler) == SIG_ERR) {
		perror("Cannot install SIGINT handler");
		return -1;
	}
	
	// start process
	tstp_handler(0);
	
	// wait infinitely
	while (1) {
		sleep(10);
	}
}
