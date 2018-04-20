#include "command_runner.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int exec_command(int argc, char **argv) {
	int to = -1;
	for (int i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "|") == 0) {
			to = i;
			argv[i] = NULL;
			break;
		}
	}

	int fd[2];
	if (to >= 0 && pipe(fd) < 0) {
		perror("Cannot create pipe");
		exit(EXIT_FAILURE);
	}
	
	pid_t child = vfork();
	
	if (child < 0) {
		perror("Cannot spawn child process");
		return -1;
	}
	
	if (child != 0) {
		if (to >= 0) {
			close(fd[1]);
			dup2(fd[0], 0);
			
			return exec_command(argc - to - 1, &argv[to + 1]);
		}
		
		return 0;
	} else {
		// child here
		
		if (to >= 0) {
			close(fd[0]);
			dup2(fd[1], 1);
		}
		
		execvp(argv[0], argv);
		
		perror("Cannot run process");
		exit(127);
	}
}

int run_command(int argc, char **argv, int line) {
	exec_command(argc, argv);
	
	int status;
	pid_t curr;
	while ((curr = waitpid(0, &status, 0)) > 0) {
		if (WEXITSTATUS(status) != 0) {
			fprintf(stderr, "Process %d exited with status: %d\n", curr, WEXITSTATUS(status));
			fprintf(stderr, "Line: %d\n", line);
			return -1;
		} else {
#ifdef DEBUG
			printf("[debug] Process %d exited normally\n", curr);
#endif
		}
	}
	
	return 0;
}
