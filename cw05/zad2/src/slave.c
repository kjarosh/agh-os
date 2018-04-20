#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>

static void print_help(char *program) {
	printf("Slave program. Use:\n");
	printf("\t%s <pipe name> <line count>\n", program);
}

static void int_handler(int t) {
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
	srand(time(NULL));
	
	if (argc != 3) {
		print_help(argv[0]);
		return -1;
	}
	
	char *pipe_name = argv[1];
	int line_count = atoi(argv[2]);
	
	int pipe_fd = open(pipe_name, O_WRONLY);
	if (pipe_fd < 0) {
		perror("Cannot open pipe");
		return -1;
	}
	
	char pid[10];
	sprintf(pid, "%d\n", (int) getpid());
	
	// handle interrupt
	signal(SIGINT, int_handler);
	
	write(pipe_fd, pid, strlen(pid));
	
	char date_buf[1024];
	char buf[1024];
	for (int i = 0; i < line_count; ++i) {
		FILE *datef = popen("date", "r");
		int r = fread(date_buf, sizeof(char), 1024, datef);
		date_buf[r] = 0;
		(void) fclose(datef);
		
		int len;
		len = sprintf(buf, "%d: %s", (int) getpid(), date_buf);
		if (len < 0) {
			fprintf(stderr, "Error printing\n");
			exit(-1);
		}
		
		(void) write(pipe_fd, buf, len);
		
		// 2--5
		sleep(rand() % 4 + 2);
	}
	
	exit(0);
}
