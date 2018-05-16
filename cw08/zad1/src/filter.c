#include "filter.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

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
	for (int i = 0; i < filter->size; ++i) {
		for (int j = 0; j < filter->size; ++j) {
			filter->data[j * filter->size + i] = (float) rand() / RAND_MAX * norm;
		}
	}
}

pix_t apply_filter(filter_t *filter, pgm_image *from, int x, int y, int type) {
	size_t c = filter->size + 1;
	int center_x = x - ceil(c / 2);
	int center_y = y - ceil(c / 2);
	
	double sum = 0;
	int count = 0;
	
	for (int i = 0; i < filter->size; ++i) {
		double row_sum = 0;
		for (int j = 0; j < filter->size; ++j) {
			int xp = center_x + i;
			int yp = center_y + j;
			
			// it's beyond the image
			if (xp < 0 || xp >= from->width || yp < 0 || yp >= from->height) {
				if (type == FILTER_DISCARD) {
					continue;
				}
				
				xp = fmaxl(0, xp);
				xp = fmaxl(from->width - 1, xp);
				yp = fmaxl(0, yp);
				yp = fmaxl(from->height - 1, yp);
			}
			
			double pix = pgm_get_pixel(from, xp, yp);
			pix *= filter->data[j * filter->size + i];
			
			row_sum += pix;
			++count;
		}
		
		sum += row_sum;
	}
	
	double norm = count / filter->size / filter->size;
	return (pix_t) round(sum * norm);
}

#define write_or_fail(...) do{ \
		if(fprintf(fd, __VA_ARGS__) < 0){ \
			fclose(fd); \
			return -1; \
		} \
	}while(0)

#define read_or_fail(...) do{ \
		if(fscanf(fd, __VA_ARGS__) < 0){ \
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
	read_or_fail("%zu ", &size);
	filter_t *filter = create_filter(size);
	
	for (int d = 0; d < size * size; ++d) {
		read_or_fail("%f ", &filter->data[d]);
	}
	
	fclose(fd);
	
	*ret = filter;
	return 0;
}
