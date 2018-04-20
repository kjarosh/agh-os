#include "command_runner.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

double tvtos(struct timeval from, struct timeval to) {
	double ret = to.tv_sec - from.tv_sec;
	ret += (to.tv_usec - from.tv_usec) / 1000000;
	return ret;
}

void set_limits(int secs, int megabytes) {
	struct rlimit rl;
	getrlimit(RLIMIT_CPU, &rl);
	getrlimit(RLIMIT_AS, &rl);
	int ra = setrlimit(RLIMIT_CPU, &(struct rlimit ) { secs, secs });
	if (ra < 0) perror("Cannot set CPU time limit");
	
	int rb = setrlimit(RLIMIT_AS,
			&(struct rlimit ) { megabytes * 1024 * 1024, megabytes * 1024 * 1024 });
	if (rb < 0) perror("Cannot set memory limit");
	
	if (ra < 0 || rb < 0) exit(127);
}

int run_command(int argc, char * const *argv, int *status, int secs, int megabytes) {
	struct rusage usage;
	
	int usage_err = 0;
	if (getrusage(RUSAGE_CHILDREN, &usage) < 0) {
		fprintf(stderr, "Couldn't get usage\n");
		usage_err = 1;
	}
	
	struct timeval startu = usage.ru_utime;
	struct timeval starts = usage.ru_stime;
	
	pid_t child = vfork();
	
	if (child < 0) {
		perror("Cannot spawn child process");
		return -1;
	}
	
	if (child == 0) {
		set_limits(secs, megabytes);
		execvp(argv[0], argv);
		
		perror("Cannot run process");
		fprintf(stderr, "When running command: %s\n", argv[0]);
		exit(127);
	} else {
		if (waitpid(child, status, 0) < 0) {
			perror("Cannot wait for child process");
			return -1;
		}
		
		if (!usage_err) {
			if (getrusage(RUSAGE_CHILDREN, &usage) < 0) {
				fprintf(stderr, "Couldn't get usage\n");
			} else {
				printf("U: %f, S: %f\n", tvtos(startu, usage.ru_utime),
						tvtos(starts, usage.ru_stime));
			}
		}
		
		return 0;
	}
}
