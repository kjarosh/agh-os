#include "filter.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>

filter_t *create_filter(int size) {
	filter_t *filter = malloc(sizeof(filter_t) + sizeof(float) * size * size);
	if (filter == NULL) return NULL;
	
	filter->size = size;
	return filter;
}

void destroy_filter(filter_t *filter) {
	free(filter);
}

void random_filter(filter_t *filter) {
	float norm = 2.f / filter->size / filter->size;
	double sum = 0.;
	for (int i = 0; i < filter->size; ++i) {
		for (int j = 0; j < filter->size; ++j) {
			float val = (float) rand() / RAND_MAX * norm;
			filter->data[j * filter->size + i] = val;
			
			sum += val;
		}
	}
	
	printf("Sum: %lf\n", sum);
	double sum2 = 0.;
	
	for (int i = 0; i < filter->size; ++i) {
		for (int j = 0; j < filter->size; ++j) {
			filter->data[j * filter->size + i] /= sum;
			sum2 += filter->data[j * filter->size + i];
		}
	}
	
	printf("After normalization: %lf\n", sum2);
}

pix_t apply_filter(filter_t *filter, pgm_image *from, int x, int y) {
	size_t c = filter->size;
	int center_x = x - ceil(c / 2);
	int center_y = y - ceil(c / 2);
	
	double sum = 0;
	
	for (int i = 0; i < filter->size; ++i) {
		double col_sum = 0.;
		for (int j = 0; j < filter->size; ++j) {
			int xp = center_x + i;
			int yp = center_y + j;
			
			xp = fmaxl(0, xp);
			xp = fminl(from->width - 1, xp);
			yp = fmaxl(0, yp);
			yp = fminl(from->height - 1, yp);
			
			double pix = (double) pgm_get_pixel(from, xp, yp);
			
			col_sum += pix * filter->data[j * c + i];
		}
		
		sum += col_sum;
	}
	
	return (pix_t) round(sum);
}

#define write_or_fail(...) do{ \
		if(fprintf(fd, __VA_ARGS__) < 0){ \
			fclose(fd); \
			return -1; \
		} \
	}while(0)

#define read_or_fail(n, ...) do{ \
		int __fscanf_ret = fscanf(fd, __VA_ARGS__); \
		if(__fscanf_ret != n){ \
			if(__fscanf_ret != EOF) errno = EINVAL; \
			fclose(fd); \
			return -1; \
		} \
	}while(0)

int write_filter(filter_t *filter, const char *filename) {
	FILE *fd = fopen(filename, "w");
	if (fd == NULL) {
		return -1;
	}
	
	write_or_fail("%zu\n", filter->size);
	
	for (int d = 0; d < filter->size * filter->size; ++d) {
		write_or_fail("%f ", filter->data[d]);
	}
	
	fclose(fd);
	return 0;
}

int read_filter(filter_t **ret, const char *filename) {
	*ret = NULL;
	FILE *fd = fopen(filename, "r");
	if (fd == NULL) {
		return -1;
	}
	
	size_t size;
	read_or_fail(1, "%zu ", &size);
	filter_t *filter = create_filter(size);
	if (filter == NULL) {
		fclose(fd);
		return -1;
	}
	
	for (int d = 0; d < size * size; ++d) {
		read_or_fail(1, "%f ", &filter->data[d]);
	}
	
	fclose(fd);
	
	*ret = filter;
	return 0;
}
