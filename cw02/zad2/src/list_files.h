#ifndef LIST_FILES_H_
#define LIST_FILES_H_

#include <time.h>

typedef enum cond {
	LT, EQ, GT
} cond_t;

struct date {
	int year;
	int month;
	int day;
};

/**
 * dir should be a resolved path
 */
void list_files(char *dir, cond_t cond, struct date date);

#endif
