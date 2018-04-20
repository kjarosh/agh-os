#include "command_runner.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

int run_command(int argc, char * const *argv, int *status) {
	pid_t child = vfork();
	
	if (child < 0) {
		perror("Cannot spawn child process");
		return -1;
	}
	
	if (child == 0) {
		execvp(argv[0], argv);
		
		perror("Cannot run process");
		fprintf(stderr, "When running command: %s\n", argv[0]);
		exit(127);
	} else {
		if (waitpid(child, status, 0) < 0) {
			perror("Cannot wait for child process");
			return -1;
		}
		
		return 0;
	}
}
