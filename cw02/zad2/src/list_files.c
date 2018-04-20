#include "list_files.h"

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void print_size(const struct stat *st) {
	printf("%8lu ", (unsigned long) st->st_size);
}

void print_last_mod(const struct stat *st) {
	struct tm *local = localtime(&st->st_mtime);
	
	printf("%04d-%02d-%02d ", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday);
}

void print_access0(mode_t test, char *iftrue) {
	if (test) {
		printf("%s", iftrue);
	} else {
		printf("-");
	}
}

void print_access(const struct stat *st) {
	mode_t mode = st->st_mode;
	
	print_access0(mode & S_IRUSR, "r");
	print_access0(mode & S_IWUSR, "w");
	print_access0(mode & S_IXUSR, "x");
	print_access0(mode & S_IRGRP, "r");
	print_access0(mode & S_IWGRP, "w");
	print_access0(mode & S_IXGRP, "x");
	print_access0(mode & S_IROTH, "r");
	print_access0(mode & S_IWOTH, "w");
	print_access0(mode & S_IXOTH, "x");
	
	printf(" ");
}

int check_time(time_t left, cond_t cond, struct date date) {
	// cannot compare time_t values, because we are comparing
	// dates, not times
	struct tm ll = *gmtime(&left);
	
	int cmp = 0;
	
	if (ll.tm_year > date.year - 1900) {
		cmp = 1;
	} else if (ll.tm_year < date.year - 1900) {
		cmp = -1;
	} else if (ll.tm_mon > date.month - 1) {
		cmp = 1;
	} else if (ll.tm_mon < date.month - 1) {
		cmp = -1;
	} else if (ll.tm_mday > date.day) {
		cmp = 1;
	} else if (ll.tm_mday < date.day) {
		cmp = -1;
	} else {
		cmp = 0;
	}
	
	switch (cond) {
	case LT:
		return cmp < 0 ? 0 : 1;
		
	case GT:
		return cmp > 0 ? 0 : 1;
		
	case EQ:
		return cmp == 0 ? 0 : 1;
		
	default:
		return 1;
	}
}

#ifndef NFTW
#include "list_files_dir.c"
#else
#include "list_files_nftw.c"
#endif
