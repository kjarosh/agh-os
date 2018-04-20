#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "operations.h"

void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s generate <file> <record count> <record size>\n", program);
	printf("\t%s sort <file> <record count> <record size> <mode>\n", program);
	printf("\t%s copy <file1> <file2> <record count> <record size> <mode>\n", program);
	printf("\n");
	printf("Where <mode> is either `lib' or `sys'.\n");
}

fmode_t parse_mode(char *m) {
	if (strcmp(m, "sys") == 0) {
		return SYS;
	} else if (strcmp(m, "lib") == 0) {
		return LIB;
	}
	
	return -1;
}

int main(int argc, char **argv) {
	char *program = argv[0];
	
	if (argc < 5) {
		print_help(program);
		return 1;
	}
	
	char *op = argv[1];
	
	if (strcmp(op, "generate") == 0) {
		if (argc != 5) {
			print_help(program);
			return 1;
		}
		
		char *filename = argv[2];
		size_t record_count = atoi(argv[3]);
		size_t record_size = atoi(argv[4]);
		
		op_generate(filename, record_count, record_size);
	} else if (strcmp(op, "sort") == 0) {
		if (argc != 6) {
			print_help(program);
			return 1;
		}
		
		char *filename = argv[2];
		size_t record_count = atoi(argv[3]);
		size_t record_size = atoi(argv[4]);
		enum mode mode = parse_mode(argv[5]);
		
		if (mode < 0) {
			print_help(program);
			return 1;
		}
		
		op_sort(filename, record_count, record_size, mode);
	} else if (strcmp(op, "copy") == 0) {
		if (argc != 7) {
			print_help(program);
			return 1;
		}
		
		char *filename = argv[2];
		char *filename2 = argv[3];
		size_t record_count = atoi(argv[4]);
		size_t record_size = atoi(argv[5]);
		enum mode mode = parse_mode(argv[6]);
		
		if (mode < 0) {
			print_help(program);
			return 1;
		}
		
		op_copy(filename, filename2, record_count, record_size, mode);
	} else {
		fprintf(stderr, "Unrecognized operation: %s\n", op);
		print_help(program);
		return 1;
	}
	
	return 0;
}
