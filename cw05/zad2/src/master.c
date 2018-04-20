#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUF_LEN (8*1024)

static void print_help(char *program) {
	printf("Slave program. Use:\n");
	printf("\t%s <pipe name>\n", program);
}

static void int_handler(int t) {
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		print_help(argv[0]);
		return -1;
	}
	
	char *pipe_name = argv[1];
	
	if (mkfifo(pipe_name, S_IRUSR | S_IWUSR) < 0) {
		perror("Cannot create pipe");
		exit(EXIT_FAILURE);
	}
	
	int pipe_fd = open(pipe_name, O_RDONLY);
	if (pipe_fd < 0) {
		perror("Cannot open pipe");
		return -1;
	}
	
	FILE *pipe_f = fdopen(pipe_fd, "r");
	
	// handle interrupt
	signal(SIGINT, int_handler);
	
	char buffer[BUF_LEN];
	
	while (1) {
		if (fgets(buffer, BUF_LEN, pipe_f) == NULL) break;
		
		printf("%s", buffer);
	}
}
