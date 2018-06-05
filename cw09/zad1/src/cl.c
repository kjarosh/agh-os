#include "cl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdbool.h>

char *trim(char *str) {
	while (*str == ' ')
		++str;
	str = strtok(str, " ");
	return str;
}

int __cl_read_config(struct cl *cl, FILE *file) {
	cl->time_limit = 0;
	cl->line_length = -1;
	cl->cmp = GT;
	cl->verbose = 0;
	
	int buf_s = 4 * 1024;
	char buf[buf_s];
	
	while (fgets(buf, buf_s, file) != NULL) {
		// skip empty lines
		if (buf[0] == 0 || buf[0] == '\n') continue;
		
		char *name = strtok(buf, "=");
		char *value = strtok(NULL, "\n");
		
		name = trim(name);
		value = trim(value);
		
		if (strcmp(name, "producers") == 0) {
			cl->producers = atoi(value);
		} else if (strcmp(name, "consumers") == 0) {
			cl->consumers = atoi(value);
		} else if (strcmp(name, "buffer-size") == 0) {
			cl->buffer_size = atoi(value);
		} else if (strcmp(name, "file") == 0) {
			strcpy(cl->filename, value);
		} else if (strcmp(name, "time-limit") == 0) {
			cl->time_limit = atoi(value);
		} else if (strcmp(name, "verbose") == 0) {
			cl->verbose = atoi(value);
		} else if (strcmp(name, "length") == 0) {
			if (value[0] == '<') {
				cl->cmp = LT;
				value++;
			} else if (value[0] == '>') {
				cl->cmp = GT;
				value++;
			} else {
				cl->cmp = EQ;
			}
			
			cl->line_length = atoi(value);
		} else {
			fprintf(stderr, "Unrecognized config: %s\n", name);
			return -1;
		}
	}
	
	return 0;
}

void __print_help(char *program) {
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\t%s\n", program);
	fprintf(stderr, "\t%s <config file>\n", program);
}

int cl_initialize(struct cl *cl, int argc, char **argv) {
	if (argc != 1 && argc != 2) {
		__print_help(argv[0]);
		return -1;
	}
	
	FILE *input;
	if (argc == 1) {
		input = stdin;
	} else {
		input = fopen(argv[1], "r");
		
		if (input == NULL) {
			perror("Cannot open file");
			return -1;
		}
	}
	
	int _exit_val = 0;
	if (__cl_read_config(cl, input) != 0) {
		_exit_val = -1;
	}
	
	if (argc != 1) {
		fclose(input);
	}
	
	return _exit_val;
}

int cl_shouldprint(struct cl *cl, char *line) {
	int res = 0;
	switch (cl->cmp) {
	case EQ:
		res = ((int) strlen(line) == cl->line_length);
		break;
	case LT:
		res = ((int) strlen(line) < cl->line_length);
		break;
	case GT:
		res = ((int) strlen(line) > cl->line_length);
		break;
	}
	
	return res;
}
