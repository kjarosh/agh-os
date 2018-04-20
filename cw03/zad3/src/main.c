#include <stdio.h>
#include <stdlib.h>

#include "interpreter.h"

void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s\n", program);
	printf("\t%s [file] <max time> <max mem>\n", program);
	printf("\n");
	printf("Where:\n");
	printf("\t[file] is the name of the file which includes commands,\n");
	printf("\t\tif no file is specified, commands are read from stdin\n");
	printf("\t<max time> is the maximum time each command is allowed to run\n");
	printf("\t<max mem> is the maximum memory each command is allowed to use\n");
}

int main(int argc, char **argv) {
	char *program = argv[0];
	FILE *fp;
	
	int secs;
	int megabytes;
	
	if (argc == 4) {
		char *file = argv[1];
		fp = fopen(file, "r");
		secs = atoi(argv[2]);
		megabytes = atoi(argv[3]);
	} else if (argc == 3) {
		fp = stdin;
		secs = atoi(argv[1]);
		megabytes = atoi(argv[2]);
	} else {
		print_help(program);
		return 1;
	}
	
	if (fp == NULL) {
		perror("Cannot open file");
		return -1;
	}
	
	return interpret_file(fp, secs, megabytes);
}
