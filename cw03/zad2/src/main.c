#include <stdio.h>
#include <stdlib.h>

#include "interpreter.h"

void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s\n", program);
	printf("\t%s <file>\n", program);
	printf("\n");
	printf("Where <file> is the name of the file which includes commands.\n");
	printf("If no file is specified, commands are read from stdin.\n");
}

int main(int argc, char **argv) {
	char *program = argv[0];
	FILE *fp;
	
	if (argc == 2) {
		char *file = argv[1];
		fp = fopen(file, "r");
	} else if (argc == 1) {
		fp = stdin;
	} else {
		print_help(program);
		return 1;
	}
	
	if (fp == NULL) {
		perror("Cannot open file");
		return -1;
	}
	
	return interpret_file(fp);
}
