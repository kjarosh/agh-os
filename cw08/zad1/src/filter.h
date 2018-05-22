#ifndef FILTER_H_
#define FILTER_H_

#include "pgm.h"

typedef struct {
	size_t size;
	float data[0];
} filter_t;

filter_t *create_filter(int size);
void destroy_filter(filter_t *filter);

void random_filter(filter_t *filter);
pix_t apply_filter(filter_t *filter, pgm_image *from, int x, int y);

int write_filter(filter_t *filter, const char *filename);
int read_filter(filter_t **filter, const char *filename);

#endif
