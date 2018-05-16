#include "pgm.h"

#include <stdio.h>

pgm_image *pgm_create(size_t width, size_t height) {
	pgm_image *image = malloc(sizeof(pgm_image) + sizeof(pix_t) * width * height);
	if (image == NULL) return NULL;
	
	image->width = width;
	image->height = height;
	return image;
}

void pgm_destroy(pgm_image *img) {
	free(img);
}

pix_t pgm_get_pixel(pgm_image *img, int x, int y) {
	return img->data[y * img->width + x];
}

void pgm_set_pixel(pgm_image *img, int x, int y, pix_t pix) {
	img->data[y * img->width + x] = pix;
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

int pgm_save(pgm_image *img, const char *filename) {
	FILE *fd = fopen(filename, "w");
	if (fd == NULL) {
		return -1;
	}
	
	write_or_fail("P2\n%zu %zu\n255\n", img->width, img->height);
	
	int d = 0;
	for (int row = 0; row < img->height; ++row) {
		for (int col = 0; col < img->width; ++col) {
			write_or_fail("%d ", pix_val(img->data[d++]));
		}
		
		write_or_fail("\n");
	}
	
	fclose(fd);
	return 0;
}

int pgm_load(pgm_image **ret, const char *filename) {
	*ret = NULL;
	FILE *fd = fopen(filename, "r");
	if (fd == NULL) {
		return -1;
	}
	
	size_t max_val;
	size_t width, height;
	read_or_fail("P2 %zu %zu %zu ", &width, &height, &max_val);
	pgm_image *img = pgm_create(width, height);
	if (img == NULL) {
		fclose(fd);
		return -1;
	}
	
	int d = 0;
	for (int row = 0; row < img->height; ++row) {
		for (int col = 0; col < img->width; ++col) {
			read_or_fail("%d ", (int * ) &img->data[d]);
			img->data[d++] *= 255.f / max_val;
		}
	}
	
	fclose(fd);
	*ret = img;
	return 0;
}
