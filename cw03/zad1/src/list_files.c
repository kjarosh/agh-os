#include "list_files.h"

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>

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

void list_file(char *filename, cond_t cond, struct date date) {
	struct stat st;
	if (lstat(filename, &st) != 0) {
		perror("lstat failed");
		return;
	}
	
	mode_t mode = st.st_mode;
	if (S_ISDIR(mode)) {
		list_files(filename, cond, date);
		return;
	}
	
	if (S_ISREG(mode)) {
		if (check_time(st.st_mtime, cond, date) != 0) {
#ifdef DEBUG
			printf("[DEBUG] %20s ", "!COND:");
#else
			return;
#endif
		}
		
		print_access(&st);
		print_last_mod(&st);
		print_size(&st);
		printf("%s\n", filename);
		
		return;
	}
	
#ifdef DEBUG
	printf("[DEBUG] %20s %s\n", "!REG && !DIR:", filename);
#endif
}

void list_files(char *dirname, cond_t cond, struct date date) {
	DIR *dir = opendir(dirname);
	if (dir == NULL) {
		perror("Cannot open directory");
		return;
	}
	
	struct dirent *dirent;
	while ((dirent = readdir(dir)) != NULL) {
		if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
			continue;
		}
		
		char path[PATH_MAX] = { 0 };
		strcat(path, dirname);
		
		if (dirname[strlen(dirname) - 1] != '/') {
			strcat(path, "/");
		}
		
		strcat(path, dirent->d_name);
		
#ifdef CLOSE_DIRS
		long int loc = telldir(dir);
		closedir(dir);
#endif
		
		pid_t pid = fork();
		
		if (pid == 0) {
			list_file(path, cond, date);
			
			// wait for all children to finish
			pid_t p;
			do {
				p = wait(NULL);
			} while (p != -1);
			
			exit(0);
		} else if (pid < 0) {
			perror("Cannot instantiate process");
			fprintf(stderr, "Continuing within the same process\n");
			list_file(path, cond, date);
		}
		
#ifdef CLOSE_DIRS
		dir = opendir(dirname);
		if (dir == NULL) {
			perror("Cannot open directory");
			return;
		}

		seekdir(dir, loc);
#endif
	}
	
	closedir(dir);
}

