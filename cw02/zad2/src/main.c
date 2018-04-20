#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdlib.h>

#include "list_files.h"

void print_help(char *program) {
	printf("Usage:\n");
	printf("\t%s <dir> <cond> <date>\n", program);
	printf("\n");
	printf("Where:\n");
	printf("\t<dir> is the directory to list files from\n");
	printf("\t<cond> one of: `<', `>' or `='; specifies the filtering condition\n");
	printf("\t<date> format yyy-mm-dd; specifies the date to compare to\n");
}

int main(int argc, char **argv) {
	if (argc != 4) {
		print_help(argv[0]);
		return 1;
	}
	
	char *dir = argv[1];
	char *condv = argv[2];
	char *date = argv[3];
	
	int year, month, day;
	int rd = sscanf(date, "%d-%d-%d", &year, &month, &day);
	
	if (rd != 3) {
		fprintf(stderr, "Error: invalid date format '%s'\n", date);
		print_help(argv[0]);
		return 2;
	}
	
	cond_t cond;
	if (strcmp(condv, "<") == 0) {
		cond = LT;
	} else if (strcmp(condv, "=") == 0) {
		cond = EQ;
	} else if (strcmp(condv, ">") == 0) {
		cond = GT;
	} else {
		fprintf(stderr, "Error: invalid condition '%s'\n", condv);
		print_help(argv[0]);
		return 3;
	}
	
	struct date sdate;
	sdate.year = year;
	sdate.month = month;
	sdate.day = day;
	
	char *dirr = realpath(dir, NULL);
#ifdef DEBUG
	printf("Resolved path: %s\n", dirr);
#endif
	if (dirr == NULL) {
		perror("Error when resolving path");
		return 5;
	}
	
	list_files(dirr, cond, sdate);
	free(dirr);
}
