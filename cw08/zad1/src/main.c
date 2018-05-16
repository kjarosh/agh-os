#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "pgm.h"

static void print_help(char *program) {
	printf("Usage:\n");
	printf("\t %s gen <size> <filter>\n", program);
	printf("or:\n");
	printf("\t %s <threads> <input> <filter> <output>\n", program);
}

int main(int argc, char **argv) {
	if (argc < 4) {
		print_help(argv[0]);
		return 1;
	}
	
	if (strcmp(argv[1], "gen") == 0) {
		if (argc != 4) {
			print_help(argv[0]);
			return 1;
		}
		
		filter_t *f = create_filter(atoi(argv[2]));
		random_filter(f);
		if (write_filter(f, argv[3]) != 0) {
			perror("Cannot write filter");
			return 1;
		}
		
		return 0;
	}
	
	if (argc != 5) {
		print_help(argv[0]);
		return 1;
	}
	
	
}
